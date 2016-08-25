const binding = require('./binding')
const stream = require('stream')

class Encoder {

	encode({
		src,
		dest,
		bitRate = 112000,
		sampleRate,
		channels
	}, callback) {
		binding.encode(src, dest, bitRate, sampleRate, channels, callback)
	}
}


module.exports = Encoder
