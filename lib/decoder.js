const binding = require('./binding')

/**
 * The decoder class, capable of decoding AAC encoded audio to PCM WAV
 * @constructor
 */
var Decoder = function() {
	return {
		decode: function(src, dest, callback) {
			binding.decode(src, dest, callback)
		}
	}

}

module.exports = Decoder
