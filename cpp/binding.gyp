{
  "targets": [
    {
      "target_name": "rainfall",
      "sources": [ "rainfall.cc" , "rainfall_node.cc" ],
      "cflags": ["-Wall", "-Wc++11-extensions", "-std=c++11"],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-std=c++11'
        ],
      },
    }
  ]
}
