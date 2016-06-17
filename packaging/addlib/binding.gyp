{
  "targets": [
    {
      "target_name": "addlib",
      "sources": [ "add.cpp", "addlib.cpp" ], 
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}