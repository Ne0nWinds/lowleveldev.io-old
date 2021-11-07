"use strict";
let memory;

const imports = {
	memory: new WebAssembly.Memory({'initial': 32}),
	_print: (src, length) => {
		if (length != 0) {
			const view = new Uint8Array(memory.buffer, src, length);
			console.log(new TextDecoder('utf-8').decode(view));
		} else {
			console.log(src);
		}
	}
};
const { instance } = await WebAssembly.instantiateStreaming(
	fetch("./compiler/build/binary.wasm"),
	{ 
		"env": imports
	}
);
memory = instance.exports.memory;
console.log(instance);
export default instance.exports;
