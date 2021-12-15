// https://webassembly.github.io/spec/core/binary/modules.html#sections
enum Section {
	SECTION_CUSTOM = 0x0,
	SECTION_TYPE,
	SECTION_IMPORT,
	SECTION_FUNC,
	SECTION_TABLE,
	SECTION_MEMORY,
	SECTION_GLOBAL,
	SECTION_EXPORT,
	SECTION_START,
	SECTION_ELEMENT,
	SECTION_CODE,
	SECTION_DATA
};

// https://webassembly.github.io/spec/core/binary/types.html
enum Valtype {
	VAL_I32 = 0x7F,
	VAL_I64 = 0x7E,
	VAL_F32 = 0x7D,
	VAL_F64 = 0x7C,

	VAL_FUNC_REF = 0x70,
	VAL_EXTERN_REF = 0x6F
};

// https://webassembly.github.io/spec/core/binary/instructions.html
enum OpCode {
	OP_GET_LOCAL = 0x20,
	OP_F32_ADD = 0x92,

	OP_DROP = 0x1A,

	OP_I32_CONST = 0x41,

	OP_I32_EQ = 0x46,
	OP_I32_NE = 0x47,
	OP_I32_LT_S = 0x48,
	OP_I32_LT_U = 0x49,
	OP_I32_GT_S = 0x4a,
	OP_I32_GT_U = 0x4b,
	OP_I32_LE_S = 0x4c,
	OP_I32_LE_U = 0x4d,
	OP_I32_GE_S = 0x4e,
	OP_I32_GE_U = 0x4f,

	OP_I32_ADD = 0x6A,
	OP_I32_SUB = 0x6B,
	OP_I32_MUL = 0x6C,
	OP_I32_DIV_S = 0x6D,
	OP_I32_DIV_U = 0x6E,

	OP_I32_LOAD = 0x28,
	OP_I32_STORE = 0x36,

	OP_NONE = 0x01,
	OP_BLOCK = 0x02,
	OP_LOOP = 0x03,
	OP_IF = 0x04,
	OP_ELSE = 0x05,
	OP_RETURN = 0x0F,
	OP_BRANCH = 0x0C,
	OP_BRANCH_IF = 0x0D,
	OP_END = 0x0B
};

// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType {
	EXPORT_FUNC = 0x0,
	EXPORT_TABLE = 0x1,
	EXPORT_MEM = 0x2,
	EXPORT_GLOBAL = 0x3,
};

#define FUNCTION_TYPE = 0x60;
#define EMPTY_ARRAY = 0x0;
