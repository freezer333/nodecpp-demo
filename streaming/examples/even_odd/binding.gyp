{
  "targets": [
    {
      "target_name": "even_odd_worker",
      "sources": [ "even_odd_worker.cpp" ], 
      "cflags": ["-Wall", "-std=c++11"],
      "include_dirs" : ["<!(node -e \"require('nan')\")", "../../dist"],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-std=c++11'
        ],
      },
      "conditions": [
        [ 'OS=="mac"', {
            "xcode_settings": {
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
                'MACOSX_DEPLOYMENT_TARGET': '10.7' }
            }
        ]
      ]
    }
  ]
}