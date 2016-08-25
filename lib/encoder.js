const binding = require('./binding')
const stream = require('stream')

class Encoder {

	encode(src, dest, options, callback) {
		if (typeof options === 'function') {
			callback = options
			options = {}
		}
		if (options.bitRate === undefined) {
			options.bitRate = 112000
		}
		binding.encode(src, dest, options.bitRate, options.sampleRate, options.channels, callback)
	}
}


module.exports = Encoder
