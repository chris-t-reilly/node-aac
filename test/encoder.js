var Encoder = require('../lib/encoder')
var path = require('path')

var e = new Encoder()

e.encode(path.join(process.cwd(), 'test/fixtures/piano.wav'), 'piano-out.aac', function() {
	console.log('Done')
})
