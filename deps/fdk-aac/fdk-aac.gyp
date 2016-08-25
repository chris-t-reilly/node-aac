{
  'variables': {
    'target_arch%': 'ia32', # built for a 32-bit CPU by default
  },
  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1, # static debug
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 0, # static release
          },
        },
      }
    },
    'msvs_settings': {
      'VCLinkerTool': {
        'GenerateDebugInformation': 'true',
      },
    },
    'conditions': [
      ['OS=="mac"', {
        'conditions': [
          ['target_arch=="ia32"', {
		  		'xcode_settings': {
				  	'ARCHS': [ 'i386' ],
					"OTHER_CPLUSPLUSFLAGS" : [ "-Wno-c++11-narrowing"],
		            "MACOSX_DEPLOYMENT_TARGET": "10.7"
				}
			}
		  ],
          ['target_arch=="x64"', {
		  		'xcode_settings': {
				  	'ARCHS': [ 'x86_64' ],
					"OTHER_CPLUSPLUSFLAGS" : [ "-Wno-c++11-narrowing"],
		            "MACOSX_DEPLOYMENT_TARGET": "10.7"
				}
			}
		  ]
        ],
      }],
    ]
  },

  'targets': [
    {
      'target_name': 'libfdk-aac',
      'product_prefix': '',
      'type': 'static_library',
      'sources': [
	    '<!@(ls -1 libFDK/src/*.cpp)',
		'<!@(ls -1 libPCMutils/src/*.cpp)',
		'<!@(ls -1 libSBRdec/src/*.cpp)',
		'<!@(ls -1 libSBRenc/src/*.cpp)',
		'<!@(ls -1 libMpegTPDec/src/*.cpp)',
		'<!@(ls -1 libMpegTPEnc/src/*.cpp)',
		'<!@(ls -1 libSYS/src/*.cpp)',
        '<!@(ls -1 libAACenc/src/*.cpp)',
		'<!@(ls -1 libAACdec/src/*.cpp)',
		'<!@(ls -1 wavTools/src/wavwriter.c)'
      ],
      'defines': [
        'PIC',
        'HAVE_CONFIG_H',
      ],
      'include_dirs': [
        'libAACdec/include',
		'libAACenc/include',
		'libFDK/include',
		'libMpegTPDec/include',
		'libMpegTPEnc/include',
		'libPCMutils/include',
		'libSBRdec/include',
		'libSBRenc/include',
		'libSYS/include',
		'wavTools/include'
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'libAACdec/include',
		  'libAACenc/include',
		  'libFDK/include',
		  'libMpegTPDec/include',
		  'libMpegTPEnc/include',
		  'libPCMutils/include',
		  'libSBRdec/include',
		  'libSBRenc/include',
		  'libSYS/include',
		  'wavTools/include'
        ],
      },
    }
  ]
}
