{
  "targets": [
    {
      "target_name": "scaling",
      "conditions": [
        ["OS=='win'", {
          "sources": [ "scaling.cc", "DpiHelper.cc" ]
        }]
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
