{
  "targets": [
    {
      "target_name": "rainfall",
      "sources": [ "rainfall.cc" , "rainfall_node.cc" ],
      'cflags_cc+': ["-Wall", "-Wc++11-extensions", "-std=c++11"],
    }
  ]
}
