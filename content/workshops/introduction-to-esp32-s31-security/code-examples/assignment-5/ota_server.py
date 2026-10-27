# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0

import http.server
import ssl
import sys


ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ctx.load_cert_chain(sys.argv[1], sys.argv[2])
srv = http.server.ThreadingHTTPServer(
    ("127.0.0.1", 8070), http.server.SimpleHTTPRequestHandler
)
srv.socket = ctx.wrap_socket(srv.socket, server_side=True)
print("Serving HTTPS on 127.0.0.1:8070")
srv.serve_forever()
