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
	OP_END = 0x0b,
	OP_GET_LOCAL = 0x20,
	OP_F32_ADD = 0x92,

	OP_I32_CONST = 0x41
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
