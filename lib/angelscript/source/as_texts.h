/*
   AngelCode Scripting Library
   Copyright (c) 2003-2015 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/


//
// as_texts.h
//
// These are text strings used through out the library
//


#ifndef AS_TEXTS_H
#define AS_TEXTS_H

// Compiler messages

#define TXT_s_ALREADY_DECLARED                      "'%s' is already declared"
#define TXT_ABSTRACT_CLASS_s_CANNOT_BE_INSTANTIATED "Abstract class '%s' cannot be instantiated"
#define TXT_ACCESSING_PRIVATE_PROP_s                "Accessing private property '%s' of parent class"
#define TXT_ARG_NOT_LVALUE                          "Output argument expression is not assignable"
#define TXT_ATTR_s_INFORMED_MULTIPLE_TIMES          "Attribute '%s' informed multiple times"
#define TXT_AUTO_NOT_ALLOWED                        "Auto is not allowed here"

#define TXT_BOTH_MUST_BE_SAME                     "Both expressions must have the same type"
#define TXT_BOTH_CONDITIONS_MUST_CALL_CONSTRUCTOR "Both conditions must call constructor"
#define TEXT_BASE_DOESNT_HAVE_DEF_CONSTR          "Base class doesn't have default constructor. Make explicit call to base constructor"

#define TXT_CANDIDATES_ARE                         "Candidates are:"
#define TXT_CANNOT_CALL_CONSTRUCTOR_IN_LOOPS       "Can't call a constructor in loops"
#define TXT_CANNOT_CALL_CONSTRUCTOR_IN_SWITCH      "Can't call a constructor in switch"
#define TXT_CANNOT_CALL_CONSTRUCTOR_TWICE          "Can't call a constructor multiple times"
#define TXT_CANNOT_CREATE_DELEGATE_FOR_NOREF_TYPES "Can't create delegate for types that do not support handles"
#define TXT_CANNOT_IMPLEMENT_SELF                  "Can't implement itself, or another interface that implements this interface"
#define TXT_CANNOT_INHERIT_FROM_s_FINAL            "Can't inherit from class '%s' marked as final"
#define TXT_CANNOT_INHERIT_FROM_MULTIPLE_CLASSES   "Can't inherit from multiple classes"
#define TXT_CANNOT_INHERIT_FROM_SELF               "Can't inherit from itself, or another class that inherits from this class"
#define TXT_CANNOT_PASS_CLASS_METHOD_AS_ARG        "Can't pass class method as arg directly. Use a delegate object instead"
#define TXT_CANNOT_RESOLVE_AUTO                     "Unable to resolve auto type"
#define TXT_CANNOT_RETURN_REF_TO_LOCAL             "Can't return reference to local value."
#define TXT_CANT_CONSTRUCT_s_USE_REF_CAST          "Can't construct handle '%s'. Use ref cast instead"
#define TXT_CANT_IMPLICITLY_CONVERT_s_TO_s         "Can't implicitly convert from '%s' to '%s'."
#define TXT_CANT_RETURN_VALUE                      "Can't return value when return type is 'void'"
#define TXT_CHANGE_SIGN                            "Implicit conversion changed sign of value"
#define TXT_CLASS_CANT_BE_FINAL_AND_ABSTRACT       "A class cannot be both abstract and final"
#define TXT_COMPILING_s                            "Compiling %s"
#define TXT_COMPOUND_ASGN_ON_VALUE_TYPE            "Compound assignments with property accessors on value types are not supported"
#define TXT_COMPOUND_ASGN_WITH_IDX_PROP            "Compound assignments with indexed property accessors are not supported"
#define TXT_COMPOUND_ASGN_REQUIRE_GET_SET          "Compound assignments with property accessors require both get and set accessors"

#define TXT_DATA_TYPE_CANT_BE_s                     "Data type can't be '%s'"
#define TXT_DECL_IN_SWITCH                          "Variables cannot be declared in switch cases, except inside statement blocks"
#define TXT_DEFAULT_MUST_BE_LAST                    "The default case must be the last one"
#define TXT_DEF_ARG_MISSING_IN_FUNC_s               "All subsequent parameters after the first default value must have default values in function '%s'"
#define TXT_DEF_ARG_TYPE_DOESNT_MATCH               "The type of the default argument expression doesn't match the function parameter type"
#define TXT_DUPLICATE_NAMED_ARG                     "Duplicate named argument"
#define TXT_DERIVED_METHOD_MUST_HAVE_SAME_RETTYPE_s "The method in the derived class must have the same return type as in the base class: '%s'"
#define TXT_DESTRUCTOR_MAY_NOT_HAVE_PARM            "The destructor must not have any parameters"
#define TXT_DESTRUCTOR_s_s_NAME_ERROR              "The name of the destructor '%s::~%s' must be the same as the class"
#define TXT_DISALLOW_ASSIGN_ON_REF_TYPE             "Value assignment on reference types is not allowed. Did you mean to do a handle assignment?"
#define TXT_DISALLOW_COMPOUND_ASSIGN_ON_REF_TYPE    "Compound assignment on reference types is not allowed"
#define TXT_DUPLICATE_SWITCH_CASE                   "Duplicate switch case"

#define TXT_ELSE_WITH_EMPTY_STATEMENT         "Else with empty statement"
#define TXT_EMPTY_LIST_ELEMENT_IS_NOT_ALLOWED "Empty list element is not allowed"
#define TXT_EMPTY_SWITCH                      "Empty switch statement"
#define TXT_EXPECTED_s                        "Expected '%s'"
#define TXT_EXPECTED_CONSTANT                 "Expected constant"
#define TXT_EXPECTED_DATA_TYPE                "Expected data type"
#define TXT_EXPECTED_EXPRESSION_VALUE         "Expected expression value"
#define TXT_EXPECTED_IDENTIFIER               "Expected identifier"
#define TXT_EXPECTED_LIST                     "Expected a list enclosed by { } to match pattern"
#define TXT_EXPECTED_METHOD_OR_PROPERTY       "Expected method or property"
#define TXT_EXPECTED_ONE_OF                   "Expected one of: "
#define TXT_EXPECTED_OPERATOR                 "Expected operator"
#define TXT_EXPECTED_s_OR_s                   "Expected '%s' or '%s'"
#define TXT_EXPECTED_POST_OPERATOR            "Expected post operator"
#define TXT_EXPECTED_PRE_OPERATOR             "Expected pre operator"
#define TXT_EXPECTED_STRING                   "Expected string"
#define TXT_EXPR_DOESNT_EVAL_TO_FUNC          "Expression doesn't evaluate to a function"
#define TXT_EXPR_MUST_BE_BOOL                 "Expression must be of boolean type"

#define TXT_FAILED_TO_COMPILE_DEF_ARG_d_IN_FUNC_s "Failed while compiling default arg for parameter %d in function '%s'"
#define TXT_FAILED_TO_CREATE_TEMP_OBJ             "Previous error occurred while attempting to create a temporary copy of object"
#define TXT_FLOAT_CONV_TO_INT_CAUSE_TRUNC         "Float value truncated in implicit conversion to integer"
#define TXT_FOUND_MULTIPLE_ENUM_VALUES            "Found multiple matching enum values"
#define TXT_FUNCTION_ALREADY_EXIST                "A function with the same name and parameters already exists"
#define TXT_FUNCTION_s_NOT_FOUND                  "Function '%s' not found"

#define TXT_GET_SET_ACCESSOR_TYPE_MISMATCH_FOR_s "The property '%s' has mismatching types for the get and set accessors"
#define TXT_GLOBAL_VARS_NOT_ALLOWED              "Global variables have been disabled by the application"

#define TXT_HANDLE_ASSIGN_ON_NON_HANDLE_PROP     "It is not allowed to perform a handle assignment on a non-handle property"
#define TXT_HANDLE_COMPARISON                    "The operand is implicitly converted to handle in order to compare them"
#define TXT_HANDLE_OF_HANDLE_IS_NOT_ALLOWED      "Handle to handle is not allowed"
#define TXT_s_HIDES_VAR_IN_OUTER_SCOPE           "Variable '%s' hides another variable of same name in outer scope"

#define TXT_IDENTIFIER_s_NOT_DATA_TYPE             "Identifier '%s' is not a data type"
#define TXT_IDENTIFIER_s_NOT_DATA_TYPE_IN_GLOBAL_NS "Identifier '%s' is not a data type in global namespace"
#define TXT_IDENTIFIER_s_NOT_DATA_TYPE_IN_NS_s     "Identifier '%s' is not a data type in namespace '%s' or parent"
#define TXT_IF_WITH_EMPTY_STATEMENT                "If with empty statement"
#define TXT_ILLEGAL_MEMBER_TYPE                    "Illegal member type"
// TODO: Should be TXT_ILLEGAL_OPERATION_ON_s
#define TXT_ILLEGAL_OPERATION                      "Illegal operation on this datatype"
#define TXT_ILLEGAL_OPERATION_ON_s                 "Illegal operation on '%s'"
#define TXT_ILLEGAL_TARGET_TYPE_FOR_REF_CAST       "Illegal target type for reference cast"
#define TXT_ILLEGAL_VARIABLE_NAME_s                "Illegal variable name '%s'."
#define TXT_INHERITED_PRIVATE_PROP_ACCESS_s        "Illegal access to inherited private property '%s'"
#define TXT_INIT_LIST_CANNOT_BE_USED_WITH_s        "Initialization lists cannot be used with '%s'"
#define TXT_INSTANCING_INVLD_TMPL_TYPE_s_s         "Attempting to instantiate invalid template type '%s<%s>'"
#define TXT_INSTEAD_FOUND_s                        "Instead found '%s'"
#define TXT_INTERFACE_s_CANNOT_BE_INSTANTIATED     "Interface '%s' cannot be instantiated"
#define TXT_INTERFACE_CAN_ONLY_IMPLEMENT_INTERFACE "Interfaces can only implement other interfaces"
#define TXT_INVALID_BREAK                          "Invalid 'break'"
#define TXT_INVALID_CHAR_LITERAL                   "Invalid character literal"
#define TXT_INVALID_CONTINUE                       "Invalid 'continue'"
#define TXT_INVALID_ESCAPE_SEQUENCE                "Invalid escape sequence"
#define TXT_INVALID_EXPRESSION_AMBIGUOUS_NAME      "Invalid expression: ambiguous name"
#define TXT_INVALID_EXPRESSION_LAMBDA              "Invalid expression: stand-alone anonymous function"
#define TXT_INVALID_OP_ON_METHOD                   "Invalid operation on method"
#define TXT_INVALID_REF_PROP_ACCESS                "Invalid reference. Property accessors cannot be used in combined read/write operations"
#define TXT_INVALID_SCOPE                          "Invalid scope resolution"
#define TXT_INVALID_TYPE                           "Invalid type"
#define TXT_INVALID_UNICODE_FORMAT_EXPECTED_d      "Invalid unicode escape sequence, expected %d hex digits"
#define TXT_INVALID_UNICODE_VALUE                  "Invalid unicode code point"
#define TXT_INVALID_UNICODE_SEQUENCE_IN_SRC        "Invalid unicode sequence in source"

#define TXT_METHOD_CANNOT_OVERRIDE_s                "Method '%s' declared as final and cannot be overridden"
#define TXT_METHOD_CANT_HAVE_NAME_OF_CLASS          "The method cannot be named with the class name"
#define TXT_METHOD_s_DOES_NOT_OVERRIDE              "Method '%s' marked as override but does not replace any base class or interface method"
#define TXT_METHOD_s_s_HAS_NO_RETURN_TYPE           "Method '%s::%s' is missing the return type, nor is it the same name as object to be a constructor"
#define TXT_MISSING_IMPLEMENTATION_OF_s             "Missing implementation of '%s'"
#define TXT_MIXIN_CANNOT_BE_DECLARED_AS_s           "Mixin class cannot be declared as '%s'"
#define TXT_MIXIN_CANNOT_HAVE_CONSTRUCTOR           "Mixin classes cannot have constructors or destructors"
#define TXT_MIXIN_CLASS_CANNOT_INHERIT              "Mixin class cannot inherit from classes"
#define TXT_MORE_THAN_ONE_MATCHING_OP               "Found more than one matching operator"
#define TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s       "Multiple matching signatures to '%s'"
#define TXT_MULTIPLE_PROP_GET_ACCESSOR_FOR_s        "Found multiple get accessors for property '%s'"
#define TXT_MULTIPLE_PROP_SET_ACCESSOR_FOR_s        "Found multiple set accessors for property '%s'"
#define TXT_MULTILINE_STRINGS_NOT_ALLOWED           "Multiline strings are not allowed in this application"
#define TXT_MUST_BE_OBJECT                          "Only objects have constructors"
#define TXT_MUST_RETURN_VALUE                       "Must return a value"

#define TXT_NAMESPACE_s_DOESNT_EXIST               "Namespace '%s' doesn't exist."
#define TXT_NAME_CONFLICT_s_EXTENDED_TYPE          "Name conflict. '%s' is an extended data type."
#define TXT_NAME_CONFLICT_s_GLOBAL_PROPERTY        "Name conflict. '%s' is a global property."
#define TXT_NAME_CONFLICT_s_IS_NAMED_TYPE          "Name conflict. '%s' is a named type."
#define TXT_NAME_CONFLICT_s_IS_FUNCDEF             "Name conflict. '%s' is a funcdef."
#define TXT_NAME_CONFLICT_s_IS_MIXIN               "Name conflict. '%s' is a mixin class."
#define TXT_NAME_CONFLICT_s_STRUCT                 "Name conflict. '%s' is a class."
#define TXT_NAME_CONFLICT_s_OBJ_PROPERTY           "Name conflict. '%s' is an object property."
#define TXT_NAME_CONFLICT_s_METHOD                 "Name conflict. '%s' is a class method."
#define TXT_NAME_CONFLICT_s_ALREADY_USED           "Name conflict. '%s' is already used."
#define TXT_NAMED_ARGS_WITH_OLD_SYNTAX             "Detected named argument with old syntax"
#define TXT_NO_APPROPRIATE_INDEX_OPERATOR          "No appropriate indexing operator found"
#define TXT_NO_APPROPRIATE_OPHNDLASSIGN_s          "No appropriate opHndlAssign method found in '%s' for handle assignment"
#define TXT_NO_APPROPRIATE_OPEQUALS                "No appropriate opEquals method found"
#define TXT_NO_CONVERSION_s_TO_s                   "No conversion from '%s' to '%s' available."
#define TXT_NO_CONVERSION_s_TO_MATH_TYPE           "No conversion from '%s' to math type available."
#define TXT_NO_DEFAULT_ARRAY_TYPE                  "The application doesn't support the default array type."
#define TXT_NO_DEFAULT_CONSTRUCTOR_FOR_s           "No default constructor for object of type '%s'."
#define TXT_NO_DEFAULT_COPY_OP_FOR_s               "No appropriate opAssign method found in '%s' for value assignment"
#define TXT_NO_COPY_CONSTRUCTOR_FOR_s              "No copy constructor for object of type '%s'."
#define TXT_NO_MATCHING_SIGNATURES_TO_s            "No matching signatures to '%s'"
#define TXT_NO_MATCHING_OP_FOUND_FOR_TYPE_s        "No matching operator that takes the type '%s' found"
#define TXT_NO_MATCHING_OP_FOUND_FOR_TYPES_s_AND_s "No matching operator that takes the types '%s' and '%s' found"
#define TXT_NON_CONST_METHOD_ON_CONST_OBJ          "Non-const method call on read-only object reference"
#define TXT_NONTERMINATED_STRING                   "Non-terminated string literal"
#define TXT_NOT_A_FUNC_s_IS_VAR                    "Expression doesn't form a function call. '%s' is a variable of a non-function type"
#define TXT_NOT_ALL_PATHS_RETURN                   "Not all paths return a value"
#define TXT_NOT_ENOUGH_VALUES_FOR_LIST             "Not enough values to match pattern"
#define TXT_s_NOT_DECLARED                         "'%s' is not declared"
#define TXT_NOT_EXACT                              "Implicit conversion of value is not exact"
#define TXT_s_NOT_INITIALIZED                      "'%s' is not initialized."
#define TXT_NOT_LVALUE                             "Expression is not an l-value"
#define TXT_s_NOT_MEMBER_OF_s                      "'%s' is not a member of '%s'"
#define TXT_NOT_VALID_REFERENCE                    "Not a valid reference"
#define TXT_NOT_VALID_LVALUE                       "Not a valid lvalue"
#define TXT_NOTHING_WAS_BUILT                      "Nothing was built in the module"

#define TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP          "Type '%s' doesn't support the indexing operator"
#define TXT_OBJECT_HANDLE_NOT_SUPPORTED             "Object handle is not supported for this type"
#define TXT_ONLY_OBJECTS_MAY_USE_REF_INOUT          "Only object types that support object handles can use &inout. Use &in or &out instead"
#define TXT_ONLY_ONE_ARGUMENT_IN_CAST               "A cast operator has one argument"
#define TXT_ONLY_ONE_FUNCTION_ALLOWED               "The code must contain one and only one function"
#define TXT_ONLY_ONE_VARIABLE_ALLOWED               "The code must contain one and only one global variable"
#define TXT_OPERANDS_MUST_BE_HANDLES                "Both operands must be handles when comparing identity"
#define TXT_OVERLOAD_CONFLICTS_DUE_TO_DEFAULT_ARGS  "The overloaded functions are identical on initial parameters without default arguments"

#define TXT_PARAMETER_ALREADY_DECLARED            "Parameter already declared"
#define TXT_PARAMETER_CANT_BE_s                   "Parameter type can't be '%s', because the type cannot be instantiated."
#define TXT_POS_ARG_AFTER_NAMED_ARG               "Positional arguments cannot be passed after named arguments"
#define TXT_PRIVATE_METHOD_CALL_s                 "Illegal call to private method '%s'"
#define TXT_PRIVATE_PROP_ACCESS_s                 "Illegal access to private property '%s'"
#define TXT_PROTECTED_METHOD_CALL_s               "Illegal call to protected method '%s'"
#define TXT_PROTECTED_PROP_ACCESS_s               "Illegal access to protected property '%s'"
#define TXT_PROPERTY_ACCESSOR_DISABLED            "Property accessors have been disabled by the application"
#define TXT_PROPERTY_ACCESSOR_MUST_BE_IMPLEMENTED "Property accessor must be implemented"
#define TXT_PROPERTY_CANT_BE_CONST                "Class properties cannot be declared as const"
#define TXT_PROPERTY_HAS_NO_GET_ACCESSOR          "The property has no get accessor"
#define TXT_PROPERTY_HAS_NO_SET_ACCESSOR          "The property has no set accessor"
#define TXT_PROPERTY_WITHOUT_ACCESSOR             "Virtual property must have at least one get or set accessor"

#define TXT_REF_CANT_BE_TO_LOCAL_VAR            "Resulting reference cannot be returned. Returned references must not refer to local variables."
#define TXT_REF_CANT_BE_RETURNED_DEFERRED_PARAM "Resulting reference cannot be returned. There are deferred arguments that may invalidate it."
#define TXT_REF_CANT_BE_RETURNED_LOCAL_VARS     "Resulting reference cannot be returned. The expression uses objects that during cleanup may invalidate it."
#define TXT_REF_IS_READ_ONLY                    "Reference is read-only"
#define TXT_REF_IS_TEMP                         "Reference is temporary"
#define TXT_REF_TYPE_CANT_BE_PASSED_BY_VAL      "Reference types cannot be passed by value in function parameters"
#define TXT_REF_TYPE_CANT_BE_RETURNED_BY_VAL    "Reference types cannot be returned by value from functions"
#define TXT_RETURN_CANT_BE_s                    "Return type can't be '%s'"

#define TXT_SHARED_CANNOT_ACCESS_NON_SHARED_VAR_s      "Shared code cannot access non-shared global variable '%s'"
#define TXT_SHARED_CANNOT_CALL_NON_SHARED_FUNC_s       "Shared code cannot call non-shared function '%s'"
#define TXT_SHARED_CANNOT_IMPLEMENT_NON_SHARED_s       "Shared type cannot implement non-shared interface '%s'"
#define TXT_SHARED_CANNOT_INHERIT_FROM_NON_SHARED_s    "Shared class cannot inherit from non-shared class '%s'"
#define TXT_SHARED_CANNOT_USE_NON_SHARED_TYPE_s        "Shared code cannot use non-shared type '%s'"
#define TXT_SHARED_s_DOESNT_MATCH_ORIGINAL             "Shared type '%s' doesn't match the original declaration in other module"
#define TXT_SECTION_IS_EMPTY                           "The script section is empty"
#define TXT_SIGNED_UNSIGNED_MISMATCH                   "Signed/Unsigned mismatch"
#define TXT_STRINGS_NOT_RECOGNIZED                     "Strings are not recognized by the application"
#define TXT_SWITCH_CASE_MUST_BE_CONSTANT               "Case expressions must be constants"
#define TXT_SWITCH_MUST_BE_INTEGRAL                    "Switch expressions must be integral numbers"

#define TXT_TMPL_s_EXPECTS_d_SUBTYPES          "Template '%s' expects %d sub type(s)"
#define TXT_TMPL_SUBTYPE_MUST_NOT_BE_READ_ONLY "Template subtype must not be read-only"
#define TXT_TOO_MANY_JUMP_LABELS               "The function has too many jump labels to handle. Split the function into smaller ones."
#define TXT_TOO_MANY_VALUES_FOR_LIST           "Too many values to match pattern"
#define TXT_TYPE_s_NOT_AVAILABLE_FOR_MODULE    "Type '%s' is not available for this module"
#define TXT_TYPE_s_NOT_TEMPLATE                "Type '%s' is not a template type"

#define TXT_UNEXPECTED_END_OF_FILE             "Unexpected end of file"
#define TXT_UNEXPECTED_TOKEN_s                 "Unexpected token '%s'"
#define TXT_UNEXPECTED_VAR_DECL                "Unexpected variable declaration"
#define TXT_UNINITIALIZED_GLOBAL_VAR_s         "Use of uninitialized global variable '%s'."
#define TXT_UNKNOWN_SCOPE_s                    "Unknown scope '%s'"
#define TXT_UNREACHABLE_CODE                   "Unreachable code"
#define TXT_UNRECOGNIZED_VIRTUAL_PROPERTY_NODE "Virtual property contains unrecognized aspect"
#define TXT_UNUSED_SCRIPT_NODE                 "Unused script node"

#define TXT_VALUE_TOO_LARGE_FOR_TYPE      "Value is too large for data type"
#define TXT_VOID_CANT_BE_OPERAND          "Void cannot be an operand in expressions"

#define TXT_WARNINGS_TREATED_AS_ERROR     "Warnings are treated as errors by the application"
#define TXT_WHILE_PARSING_ARG_LIST        "While parsing argument list"
#define TXT_WHILE_PARSING_EXPRESSION      "While parsing expression"
#define TXT_WHILE_PARSING_INIT_LIST       "While parsing initialization list"
#define TXT_WHILE_PARSING_NAMESPACE       "While parsing namespace"
#define TXT_WHILE_PARSING_STATEMENT_BLOCK "While parsing statement block"
#define TXT_WHILE_INCLUDING_MIXIN         "Previous error occurred while including mixin"

// Global variable initialization

#define TXT_FAILED_TO_INITIALIZE_s      "Failed to initialize global variable '%s'"
#define TXT_EXCEPTION_s_IN_s            "Exception '%s' in '%s'"

// Engine message

#define TXT_AUTOHANDLE_CANNOT_BE_USED_FOR_NOCOUNT        "Autohandles cannot be used with types that have been registered with NOCOUNT"
#define TXT_FIRST_PARAM_MUST_BE_REF_FOR_TEMPLATE_FACTORY "First parameter to template factory must be a reference. This will be used to pass the object type of the template"
#define TXT_INVALID_CONFIGURATION                        "Invalid configuration. Verify the registered application interface."
#define TXT_VALUE_TYPE_MUST_HAVE_SIZE                    "A value type must be registered with a non-zero size"
#define TXT_TYPE_s_IS_MISSING_BEHAVIOURS                 "Type '%s' is missing behaviours"
#define TXT_ILLEGAL_BEHAVIOUR_FOR_TYPE                   "The behaviour is not compatible with the type"
#define TXT_GC_REQUIRE_ADD_REL_GC_BEHAVIOUR              "A garbage collected type must have the addref, release, and all gc behaviours"
#define TXT_SCOPE_REQUIRE_REL_BEHAVIOUR                  "A scoped reference type must have the release behaviour"
#define TXT_REF_REQUIRE_ADD_REL_BEHAVIOUR                "A reference type must have the addref and release behaviours"
#define TXT_NON_POD_REQUIRE_CONSTR_DESTR_BEHAVIOUR       "A non-pod value type must have the default constructor and destructor behaviours"
#define TXT_CANNOT_PASS_TYPE_s_BY_VAL                    "Can't pass type '%s' by value unless the application type is informed in the registration"
#define TXT_CANNOT_RET_TYPE_s_BY_VAL                     "Can't return type '%s' by value unless the application type is informed in the registration"
// TODO: Should be something like "This platform requires that AngelScript knows the exact content of the type '%s' in order to pass by value to application in native calling convention"
#define TXT_DONT_SUPPORT_TYPE_s_BY_VAL                   "Don't support passing type '%s' by value to application in native calling convention on this platform"
// TODO: Should be something like "This platform requires that AngelScript knows the exact content of the type '%s' in order to return by value from application in native calling convention"
#define TXT_DONT_SUPPORT_RET_TYPE_s_BY_VAL               "Don't support returning type '%s' by value from application in native calling convention on this platform"
#define TXT_d_GC_CANNOT_FREE_OBJ_OF_TYPE_s               "Object {%d}. GC cannot destroy an object of type '%s' as it doesn't know how many references to there are."
#define TXT_d_GC_CANNOT_FREE_OBJ_OF_TYPE_s_REF_COUNT_d   "Object {%d}. GC cannot destroy an object of type '%s' as it can't see all references. Current ref count is %d."
#define TXT_OBJECT_TYPE_s_DOESNT_EXIST                   "Object type '%s' doesn't exist"
#define TXT_TEMPLATE_s_ALREADY_GENERATED_CANT_REGISTER   "Cannot register. The template type instance '%s' has already been generated."
#define TXT_TEMPLATE_TYPE_s_DOESNT_EXIST                 "Template type '%s' doesn't exist"
#define TXT_TEMPLATE_SUBTYPE_s_DOESNT_EXIST              "Template subtype '%s' doesn't exist"
#define TXT_TEMPLATE_LIST_FACTORY_EXPECTS_2_REF_PARAMS   "Template list factory expects two reference parameters. The last is the pointer to the initialization buffer"
#define TXT_LIST_FACTORY_EXPECTS_1_REF_PARAM             "List factory expects only one reference parameter. The pointer to the initialization buffer will be passed in this parameter"
#define TXT_FAILED_READ_SUBTYPE_OF_TEMPLATE_s            "Failed to read subtype of template type '%s'"
#define TXT_FAILED_IN_FUNC_s_d                           "Failed in call to function '%s' (Code: %d)"
#define TXT_FAILED_IN_FUNC_s_WITH_s_d                    "Failed in call to function '%s' with '%s' (Code: %d)"
#define TXT_FAILED_IN_FUNC_s_WITH_s_AND_s_d              "Failed in call to function '%s' with '%s' and '%s' (Code: %d)"
#define TXT_GC_RECEIVED_NULL_PTR                         "AddScriptObjectToGC called with null pointer"
#define TXT_EXCEPTION_IN_NESTED_CALL                     "An exception occurred in a nested call"
#define TXT_TYPE_s_IS_STILL_USED_BY_FUNC_s               "Type '%s' is still used by function '%s'"
#define TXT_PREV_TYPE_IS_NAMED_s                         "The builtin type in previous message is named '%s'"
#define TXT_PREV_FUNC_IS_NAMED_s_TYPE_IS_d               "The function in previous message is named '%s'. The func type is %d"
#define TXT_RESURRECTING_SCRIPTOBJECT_s                  "The script object of type '%s' is being resurrected illegally during destruction"
#define TXT_INVALID_BYTECODE_d                           "LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: %d"
#define TXT_NO_JIT_IN_FUNC_s                             "Function '%s' appears to have been compiled without JIT entry points"
#define TXT_ENGINE_REF_COUNT_ERROR_DURING_SHUTDOWN       "Uh oh! The engine's reference count is increasing while it is being destroyed. Make sure references needed for clean-up are immediately released"
#define TXT_MODULE_IS_IN_USE                             "The module is still in use and cannot be rebuilt. Discard it and request another module"
#define TXT_EXTRNL_REF_TO_MODULE_s                       "There is an external reference to an object in module '%s', preventing it from being deleted"

// Internal names

#define TXT_PROPERTY                      "Property"
#define TXT_SYSTEM_FUNCTION               "System function"
#define TXT_VARIABLE_DECL                 "Variable declaration"

// Exceptions

#define TXT_STACK_OVERFLOW                "Stack overflow"
#define TXT_NULL_POINTER_ACCESS           "Null pointer access"
#define TXT_DIVIDE_BY_ZERO                "Divide by zero"
#define TXT_DIVIDE_OVERFLOW               "Overflow in integer division"
#define TXT_POW_OVERFLOW                  "Overflow in exponent operation"
#define TXT_UNRECOGNIZED_BYTE_CODE        "Unrecognized byte code"
#define TXT_INVALID_CALLING_CONVENTION    "Invalid calling convention"
#define TXT_UNBOUND_FUNCTION              "Unbound function called"
#define TXT_OUT_OF_BOUNDS                 "Out of range"
#define TXT_EXCEPTION_CAUGHT              "Caught an exception from the application"
#define TXT_MISMATCH_IN_VALUE_ASSIGN      "Mismatching types in value assignment"

#endif
