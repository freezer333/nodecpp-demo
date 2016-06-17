{
  "targets": [
    {
      "target_name": "cpp11",
      "sources": [ "hello11.cpp" ], 
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}