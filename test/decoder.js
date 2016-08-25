var Decoder = require('../lib/decoder')
var path = require('path')

var d = new Decoder()

d.decode({
	src: path.join(process.cwd(), 'test/fixtures/piano.aac'),
	dest: 'piano-out.wav'
}, function() {
	console.log('Done')
})
