const binding = require('./binding')
const stream = require('stream')

class Decoder {

	decode({
		src,
		dest
	}, callback) {
		console.log(src, callback)
		console.log(binding.decode)
		binding.decode(src, dest, callback)
	}
}

module.exports = Decoder
