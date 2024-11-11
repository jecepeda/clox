package suite

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
)

var (
	baseDir    = "test"     // directory where we are going to scan the files
	binaryFile = "bin/clox" // binary file to interpret the lox code
	// regex for expectations
	expectedOutputPattern       = regexp.MustCompile(`// expect: ?(.*)`) // regex for the expected output
	expectedErrorPattern        = regexp.MustCompile(`// (Error.*)`)
	expectedErrorLinePattern    = regexp.MustCompile(`// \[line (\d+)\] (Error.*)`)
	expectedRuntimeErrorPattern = regexp.MustCompile(`// expect runtime error: (.+)`)
	// regex for the actual output
	stackTracePattern  = regexp.MustCompile(`\[line (\d+)\]`)
	syntaxErrorPattern = regexp.MustCompile(`\[.*line (\d+)\] (Error.+)`)
)

type Config int

const (
	Unknown Config = iota
	Pass
	Skip
)

type PathConfig map[string]Config

type Suite struct {
	PathConfig PathConfig

	TestFiles []*TestFile
}

func NewSuite(pathConfig map[string]Config) *Suite {
	return &Suite{
		PathConfig: pathConfig,
	}
}

func (s *Suite) Run() error {
	err := filepath.Walk(baseDir, func(path string, info os.FileInfo, err error) error {
		if info.IsDir() {
			if strings.Contains(path, "benchmark") {
				return filepath.SkipDir
			}
			return nil
		}

		var builder strings.Builder
		var config Config
		splitted := strings.Split(path, "/")
		// we are going to iterate over the path to find the configuration
		for _, v := range splitted {
			if builder.Len() > 0 {
				builder.WriteString("/")
			}
			builder.WriteString(v)
			c, ok := s.PathConfig[builder.String()]
			if ok {
				config = c
			}
		}
		if config == Unknown {
			return fmt.Errorf("no configuration found for %s", path)
		}
		if config == Skip {
			return nil
		}

		// we can assume that the file is valid to test
		s.TestFiles = append(s.TestFiles, NewTestFile(path))

		return nil
	})
	if err != nil {
		return err
	}

	for _, tf := range s.TestFiles {
		_, err := tf.Parse()
		if err != nil {
			return fmt.Errorf("error parsing %s: %w", tf, err)
		}
		err = tf.Run()
		if err != nil {
			return fmt.Errorf("error running %s: %w", tf, err)
		}
	}

	return nil
}

type ExpectedOutPut struct {
	Line   int
	Output string
}

func (e ExpectedOutPut) String() string {
	return fmt.Sprintf("Line: %d, Output: %s", e.Line, e.Output)
}

type TestFile struct {
	Path           string
	ExpectedOutPut []ExpectedOutPut
	ExpectedErrors []string

	ExpectedRuntimeError *string
	RuntimeErrorLine     int
	ExpectedExitCode     int

	Expectations int

	Failures []string
}

func NewTestFile(path string) *TestFile {
	return &TestFile{
		Path: path,
	}
}

func (tf TestFile) String() string {
	return tf.Path
}

func (tf TestFile) PrintExpectations() {
	fmt.Printf("File: %s\n", tf.Path)
	if len(tf.ExpectedOutPut) == 0 && len(tf.ExpectedErrors) == 0 && tf.ExpectedRuntimeError == nil {
		fmt.Println("No expectations found")
		return
	}
	for _, v := range tf.ExpectedOutPut {
		fmt.Printf("Line: %d, Output: %s\n", v.Line, v.Output)
	}
	for _, v := range tf.ExpectedErrors {
		fmt.Printf("Error: %s\n", v)
	}
	if tf.ExpectedRuntimeError != nil {
		fmt.Printf("Runtime Error: %s\n", *tf.ExpectedRuntimeError)
	}
}

func (tf *TestFile) CheckExitCode(exitCode int) {
	if exitCode != tf.ExpectedExitCode {
		tf.Failures = append(
			tf.Failures,
			fmt.Sprintf("Expected exit code %d, but got %d", tf.ExpectedExitCode, exitCode),
		)
	}
}

func (tf *TestFile) CheckErrors(stderr string) {
	if tf.ExpectedRuntimeError != nil {
		tf.CheckRuntimeError(stderr)
	} else {
		tf.CheckCompileErrors(stderr)
	}
}

func (tf *TestFile) CheckCompileErrors(stderr string) {
	lines := splitOutput(stderr)
	foundErrors := make([]string, 0)
	for i := 0; i < len(lines); i++ {
		if syntaxErrorPattern.MatchString(lines[i]) {
			matches := syntaxErrorPattern.FindStringSubmatch(lines[i])
			foundErrors = append(foundErrors, fmt.Sprintf("[%s] %s", matches[1], matches[2]))
		} else {
			tf.Failures = append(
				tf.Failures,
				fmt.Sprintf("Unexpected error: %q", lines[i]),
			)
		}
	}
	for _, expected := range tf.ExpectedErrors {
		found := false
		for _, actual := range foundErrors {
			if expected == actual {
				found = true
				break
			}
		}
		if !found {
			tf.Failures = append(
				tf.Failures,
				fmt.Sprintf("Expected compile error %q, but got nothing", expected),
			)
		}
	}
}

func (tf *TestFile) CheckRuntimeError(stderr string) {
	lines := splitOutput(stderr)

	if len(lines) == 0 {
		tf.Failures = append(
			tf.Failures,
			fmt.Sprintf(
				"Expected runtime error %q, but got nothing",
				*tf.ExpectedRuntimeError,
			),
		)
		return
	}

	if lines[0] != *tf.ExpectedRuntimeError {
		tf.Failures = append(
			tf.Failures,
			fmt.Sprintf(
				"Expected runtime error %s, but got %s",
				*tf.ExpectedRuntimeError,
				lines[0],
			),
		)
		return
	}

	matched := false
	for i := 1; i < len(lines); i++ {
		// match and get the line number, it should be equal to runtime error line
		if stackTracePattern.MatchString(lines[i]) {
			matches := stackTracePattern.FindStringSubmatch(lines[i])
			if strconv.Itoa(tf.RuntimeErrorLine) == matches[1] {
				matched = true
				break
			}
		}
	}
	if !matched {
		tf.Failures = append(
			tf.Failures,
			fmt.Sprintf("Expected stack trace at line %d, but got nothing", tf.RuntimeErrorLine),
		)
	}

}

func (tf *TestFile) CheckOutput(output string) {
	lines := splitOutput(output)
	i := 0
	for ; i < len(lines); i++ {
		if i >= len(tf.ExpectedOutPut) {
			tf.Failures = append(
				tf.Failures,
				fmt.Sprintf("Unexpected output: %s", lines[i]),
			)
		} else if tf.ExpectedOutPut[i].Output != lines[i] {
			tf.Failures = append(
				tf.Failures,
				fmt.Sprintf("Expected output %q, but got %q",
					tf.ExpectedOutPut[i].Output, lines[i]),
			)
		}
	}
	if i < len(tf.ExpectedOutPut) {
		tf.Failures = append(
			tf.Failures,
			fmt.Sprintf(
				"Expected output: \n%s\nBut got nothing",
				tf.ExpectedOutPut[i],
			),
		)
	}
}

func splitOutput(output string) []string {
	splitted := strings.Split(output, "\n")
	return splitted[:len(splitted)-1]
}

func (tf *TestFile) Parse() (bool, error) {
	content, err := os.ReadFile(tf.Path)
	if err != nil {
		return false, err
	}
	lines := strings.Split(string(content), "\n")
	for i, line := range lines {
		if expectedOutputPattern.MatchString(line) {
			matches := expectedOutputPattern.FindStringSubmatch(line)
			tf.ExpectedOutPut = append(tf.ExpectedOutPut, ExpectedOutPut{
				Line:   i + 1,
				Output: matches[1],
			})
			tf.Expectations++
		}
		if expectedErrorPattern.MatchString(line) {
			matches := expectedErrorPattern.FindStringSubmatch(line)
			tf.ExpectedErrors = append(tf.ExpectedErrors, fmt.Sprintf("[%d] %s", i+1, matches[1]))
			tf.ExpectedExitCode = 65
			tf.Expectations++
		}
		if expectedRuntimeErrorPattern.MatchString(line) {
			matches := expectedRuntimeErrorPattern.FindStringSubmatch(line)
			tf.ExpectedRuntimeError = &matches[1]
			tf.RuntimeErrorLine = i + 1
			tf.ExpectedExitCode = 70
			tf.Expectations++
		}
		if expectedErrorLinePattern.MatchString(line) {
			matches := expectedErrorLinePattern.FindStringSubmatch(line)
			tf.ExpectedErrors = append(tf.ExpectedErrors, fmt.Sprintf("[%s] %s", matches[1], matches[2]))
			tf.ExpectedExitCode = 65
			tf.Expectations++
		}
	}

	return true, nil
}

func (tf *TestFile) Run() error {
	cmd := exec.Command(binaryFile, tf.Path)

	var stdout, stderr strings.Builder
	cmd.Stdout = &stdout
	cmd.Stderr = &stderr

	err := cmd.Start()
	if err != nil {
		return err
	}

	// we don't capture the error
	// to handle the runtime/compile output ourselves
	cmd.Wait()
	exitCode := cmd.ProcessState.ExitCode()

	tf.CheckOutput(stdout.String())
	tf.CheckExitCode(exitCode)
	tf.CheckErrors(stderr.String())

	if len(tf.Failures) > 0 {
		fmt.Println("File:", tf.Path)
		fmt.Println("Failures:")
		for _, v := range tf.Failures {
			fmt.Println(v)
		}
	}

	return nil
}
