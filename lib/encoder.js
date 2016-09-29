const binding = require('./binding')

var Encoder = function() {
	return {
		encode: function(src, dest, options, callback) {
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
}


module.exports = Encoder
