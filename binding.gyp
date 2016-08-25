{
  'targets': [
    {
	  'type': 'shared_library',
	  'product_extension': 'node',
	  'defines': [ 'BUILDING_NODE_EXTENSION' ],
      'target_name': 'aac',
	  'include_dirs': [ "<!(node -e \"require('nan')\")" ],
      'sources': [
	    'src/binding.cc'
	  ],
	  'dependencies': [
	  	'deps/fdk-aac/fdk-aac.gyp:libfdk-aac'
	  ],
	   'conditions': [
        ['OS=="mac"', {
          'libraries': [
            '-Wl,-force_load,<(module_root_dir)/build/$(BUILDTYPE)/fdk-aac.a',
			'-lssl'
          ],
          'xcode_settings': {
            'OTHER_LDFLAGS': [
              '-undefined dynamic_lookup'
            ],
          },
        }],
      ],
    }
  ]
}
