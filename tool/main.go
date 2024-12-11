package main

import (
	"fmt"
	"log"
	"log/slog"
	"slices"

	"github.com/jecepeda/clox/tool/suite"
)

type Filemap map[string]suite.Config

func (f Filemap) Copy(other Filemap) Filemap {
	for k, v := range other {
		f[k] = v
	}
	return f
}

var suites map[string]*suite.Suite

func main() {
	slog.SetLogLoggerLevel(slog.LevelDebug)

	suites = map[string]*suite.Suite{
		"foo": suite.NewSuite(map[string]suite.Config{
			"foo": suite.Skip,
		}),
	}
	earlyChapters := Filemap{
		"test/scanning":    suite.Skip,
		"test/expressions": suite.Skip,
	}

	noControlFlow := Filemap{
		"test/block/empty.lox":                  suite.Skip,
		"test/for":                              suite.Skip,
		"test/if":                               suite.Skip,
		"test/limit/loop_too_large.lox":         suite.Skip,
		"test/logical_operator":                 suite.Skip,
		"test/variable/unreached_undefined.lox": suite.Skip,
		"test/while":                            suite.Skip,
	}

	noFunctions := Filemap{
		"test/call":                                suite.Skip,
		"test/closure":                             suite.Skip,
		"test/for/closure_in_body.lox":             suite.Skip,
		"test/for/return_closure.lox":              suite.Skip,
		"test/for/return_inside.lox":               suite.Skip,
		"test/for/syntax.lox":                      suite.Skip,
		"test/function":                            suite.Skip,
		"test/limit/no_reuse_constants.lox":        suite.Skip,
		"test/limit/stack_overflow.lox":            suite.Skip,
		"test/limit/too_many_constants.lox":        suite.Skip,
		"test/limit/too_many_locals.lox":           suite.Skip,
		"test/limit/too_many_upvalues.lox":         suite.Skip,
		"test/regression/40.lox":                   suite.Skip,
		"test/return":                              suite.Skip,
		"test/unexpected_character.lox":            suite.Skip,
		"test/variable/collide_with_parameter.lox": suite.Skip,
		"test/variable/duplicate_parameter.lox":    suite.Skip,
		"test/variable/early_bound.lox":            suite.Skip,
		"test/while/closure_in_body.lox":           suite.Skip,
		"test/while/return_closure.lox":            suite.Skip,
		"test/while/return_inside.lox":             suite.Skip,
	}

	noClasses := Filemap{
		"test/assignment/to_this.lox":                  suite.Skip,
		"test/call/object.lox":                         suite.Skip,
		"test/class":                                   suite.Skip,
		"test/closure/close_over_method_parameter.lox": suite.Skip,
		"test/constructor":                             suite.Skip,
		"test/field":                                   suite.Skip,
		"test/inheritance":                             suite.Skip,
		"test/method":                                  suite.Skip,
		"test/number/decimal_point_at_eof.lox":         suite.Skip,
		"test/number/trailing_dot.lox":                 suite.Skip,
		"test/operator/equals_class.lox":               suite.Skip,
		"test/operator/equals_method.lox":              suite.Skip,
		"test/operator/not.lox":                        suite.Skip,
		"test/operator/not_class.lox":                  suite.Skip,
		"test/regression/394.lox":                      suite.Skip,
		"test/return/in_method.lox":                    suite.Skip,
		"test/super":                                   suite.Skip,
		"test/this":                                    suite.Skip,
		"test/variable/local_from_method.lox":          suite.Skip,
	}

	// No inheritance in C yet.
	noInheritance := Filemap{
		"test/class/local_inherit_other.lox": suite.Skip,
		"test/class/local_inherit_self.lox":  suite.Skip,
		"test/class/inherit_self.lox":        suite.Skip,
		"test/class/inherited_method.lox":    suite.Skip,
		"test/inheritance":                   suite.Skip,
		"test/regression/394.lox":            suite.Skip,
		"test/super":                         suite.Skip,
	}

	// removed early chapters as we have a whole interpreter now
	suites = map[string]*suite.Suite{
		// "clox": suite.NewSuite(Filemap{
		// 	"test": suite.Pass,
		// }.Copy(earlyChapters)),

		"chap21_global": suite.NewSuite(Filemap{
			"test": suite.Pass,

			// No blocks.
			"test/assignment/local.lox":                         suite.Skip,
			"test/variable/in_middle_of_block.lox":              suite.Skip,
			"test/variable/in_nested_block.lox":                 suite.Skip,
			"test/variable/scope_reuse_in_different_blocks.lox": suite.Skip,
			"test/variable/shadow_and_local.lox":                suite.Skip,
			"test/variable/undefined_local.lox":                 suite.Skip,

			// No local variables.
			"test/block/scope.lox":                       suite.Skip,
			"test/variable/duplicate_local.lox":          suite.Skip,
			"test/variable/shadow_global.lox":            suite.Skip,
			"test/variable/shadow_local.lox":             suite.Skip,
			"test/variable/use_local_in_initializer.lox": suite.Skip,
		}.Copy(earlyChapters).Copy(noControlFlow).Copy(noFunctions).Copy(noClasses)),

		"chap22_local": suite.NewSuite(Filemap{
			"test": suite.Pass,
		}.Copy(earlyChapters).Copy(noControlFlow).Copy(noFunctions).Copy(noClasses)),

		"chap23_jumping": suite.NewSuite(Filemap{
			"test": suite.Pass,
		}.Copy(earlyChapters).Copy(noFunctions).Copy(noClasses)),

		"chap24_calls": suite.NewSuite(Filemap{
			"test": suite.Pass,
			// No closures.
			"test/closure":                      suite.Skip,
			"test/for/closure_in_body.lox":      suite.Skip,
			"test/for/return_closure.lox":       suite.Skip,
			"test/function/local_recursion.lox": suite.Skip,
			"test/limit/too_many_upvalues.lox":  suite.Skip,
			"test/regression/40.lox":            suite.Skip,
			"test/while/closure_in_body.lox":    suite.Skip,
			"test/while/return_closure.lox":     suite.Skip,
		}.Copy(earlyChapters).Copy(noClasses)),

		"chap25_closures": suite.NewSuite(Filemap{
			"test": suite.Pass,
		}.Copy(earlyChapters).Copy(noClasses)),

		"chap26_garbage": suite.NewSuite(Filemap{
			"test": suite.Pass,
		}.Copy(earlyChapters).Copy(noClasses)),

		"chap27_classes": suite.NewSuite(Filemap{
			"test": suite.Pass,
			// No methods.
			"test/assignment/to_this.lox":                  suite.Skip,
			"test/class/local_reference_self.lox":          suite.Skip,
			"test/class/reference_self.lox":                suite.Skip,
			"test/closure/close_over_method_parameter.lox": suite.Skip,
			"test/constructor":                             suite.Skip,
			"test/field/get_and_set_method.lox":            suite.Skip,
			"test/field/method.lox":                        suite.Skip,
			"test/field/method_binds_this.lox":             suite.Skip,
			"test/method":                                  suite.Skip,
			"test/operator/equals_class.lox":               suite.Skip,
			"test/operator/equals_method.lox":              suite.Skip,
			"test/return/in_method.lox":                    suite.Skip,
			"test/this":                                    suite.Skip,
			"test/variable/local_from_method.lox":          suite.Skip,
		}.Copy(earlyChapters).Copy(noInheritance)),

		"chap28_methods": suite.NewSuite(Filemap{
			"test": suite.Pass,
		}.Copy(earlyChapters).Copy(noInheritance)),

		"chap29_superclasses": suite.NewSuite(Filemap{
			"test": suite.Pass,
		}.Copy(earlyChapters)),

		// "chap30_optimization": suite.NewSuite(Filemap{
		// 	"test": suite.Pass,
		// }.Copy(earlyChapters)),
	}

	// tedious, but to run all the tests we need to get all the keys and sort them
	keys := make([]string, 0, len(suites))
	for k := range suites {
		keys = append(keys, k)
	}
	slices.Sort(keys)

	for _, k := range keys {
		fmt.Printf("Running suite %s\n", k)
		if err := suites[k].Run(); err != nil {
			log.Fatalf("Error running suite %s: %v\n", k, err)
		}
	}
}
