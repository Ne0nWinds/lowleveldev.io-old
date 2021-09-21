import compiler from "./compiler.js"

const compileButton = document.getElementById("compile_button");
const editor = document.getElementById("editor");

compileButton.onclick = async () => {

	const { value } = editor;
	const encoder = new TextEncoder('utf-8');
	const addr = compiler.get_mem_addr();

	const view = new Uint8Array(compiler.memory.buffer, addr, value.length + 1);
	encoder.encodeInto(value, view);
	view[value.length] = 0;

	const len = compiler.compile();
	const view2 = new Uint8Array(compiler.memory.buffer, compiler.get_compiled_code(), len);
	console.log(view2);

	const { instance } = await WebAssembly.instantiate(view2);
	console.log("=== Compilation Successful ===");
	console.log(instance.exports.main());
}
