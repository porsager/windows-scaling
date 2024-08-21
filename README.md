# ⚖️ Windows Scaling

For some reason it has to be this hard to set scale programatically on Windows 🤢

This is based on the amazing work in https://github.com/lihas/windows-DPI-scaling-sample who figoured out how!

# Usage

```bash
npm i windows-scaling
```

```js
import scaling from 'windows-scaling'

// Get current scaling
scaling.get()

// Set current scaling
scaling.set(200)
```
