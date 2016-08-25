var Encoder = require('../lib/encoder')
var path = require('path')

var e = new Encoder()

e.encode({
	src: path.join(process.cwd(), 'test/fixtures/piano.wav'),
	dest: 'piano-out.aac'
}, function() {
	console.log('Done')
})
