var Decoder = require('../lib/decoder')
var path = require('path')

var d = new Decoder()

d.decode(path.join(process.cwd(), 'test/fixtures/piano.aac'), 'piano-out.wav', function() {
	console.log('Done')
})
