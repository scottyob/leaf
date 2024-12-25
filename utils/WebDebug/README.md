
Following this guide:
https://nagix.github.io/chartjs-plugin-streaming/latest/guide/getting-started.html

To get started:

```
python -m http.server -d utils/WebDebug
```

Then point your web browser to the server
Point your server to the Leaf


---
Experimenting through serial:

```
(env) [scott@sob-desktop leaf]$ stty -F /dev/ttyACM0 cs8 -cstopb -parenb 1500000
(env) [scott@sob-desktop leaf]$ cat < /dev/ttyACM0 
$ python utils/WebDebug/webserver.py < /dev/ttyACM0
```