const binding = require('./binding')
const stream = require('stream')

class Decoder {

	decode(src, dest, callback) {
		binding.decode(src, dest, callback)
	}
}

module.exports = Decoder
