{
	'variables': {
		'conditions': [
			['OS == "win"', {
				'lp%': '<!(echo %LIBPROTOBUF%)'
			}],
			['OS != "win"', {
				'lp%': '<!(sh lp.sh)'
			}]
		]
	},
	'targets': [{
		'target_name': 'protobuf',
		'sources': [ './src/init.cpp', './src/native.cpp', './src/parse.cpp', './src/serialize.cpp' ],
		'include_dirs': [
			'<!(node -e "require(\'nan\')")',
			'<(lp)/include'
		],
		'libraries': [
			'-L<(lp)/lib'
		],
		'cflags' : [ '-Ofast', '-ffast-math', '-funroll-loops', '-fomit-frame-pointer', '-std=c++11', '-pthread', '-static', '-I../../' ],
		'cflags_cc' : [ '-Ofast', '-ffast-math', '-funroll-loops', '-fomit-frame-pointer', '-std=c++11', '-pthread', '-static', '-I../../' ],
		'conditions': [
			['OS == "win"', {
				'libraries': [
					'-llibprotobuf.lib'
				],
				'msvs_settings': {
					'VCLinkerTool': {
						'AdditionalLibraryDirectories': '<(lp)/lib'
					}
				}
			}],
			['OS == "mac"', {
				'libraries': [
					'-lprotobuf'
				],
				'xcode_settings': {
					'MACOSX_DEPLOYMENT_TARGET': '10.7',
					'OTHER_CPLUSPLUSFLAGS': [
						'-stdlib=libc++',
					],
					'GCC_ENABLE_CPP_RTTI': 'YES'
				}
			}],
			['OS == "linux"', {
				'cflags_cc!': [ '-fno-rtti' ],
				'libraries': [
					'-lprotobuf'
				]
			}]
		]
	}]
}
