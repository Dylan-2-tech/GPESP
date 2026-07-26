#ifndef CERTS_H
#define CERTS_H

// Self-signed certificate + private key for the ESP32 HTTPS server.
//
// Generated once, offline, with:
//   openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem \
//     -days 3650 -nodes -subj "/CN=esp32.local/O=BikeRoute/C=FR"
//
// Valid for 10 years (until 2036). Baked into the firmware instead of
// generated on the ESP32 at boot (unlike the old fhessel library), so
// there's no startup delay and the certificate stays identical across
// reboots/reflashes.
//
// Your browser will still show a "not trusted" warning the first time you
// visit, since it's self-signed rather than issued by a public CA - that's
// expected. Choose Advanced/Proceed once and it won't ask again.

const char servercert_pem[] = R"CERT(
-----BEGIN CERTIFICATE-----
MIIDTzCCAjegAwIBAgIUCdUZFxPBfolsugbPXXUZuyDsRLQwDQYJKoZIhvcNAQEL
BQAwNzEUMBIGA1UEAwwLZXNwMzIubG9jYWwxEjAQBgNVBAoMCUJpa2VSb3V0ZTEL
MAkGA1UEBhMCRlIwHhcNMjYwNzI2MDEwMTU4WhcNMzYwNzIzMDEwMTU4WjA3MRQw
EgYDVQQDDAtlc3AzMi5sb2NhbDESMBAGA1UECgwJQmlrZVJvdXRlMQswCQYDVQQG
EwJGUjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALEZn/Yl36pfhM87
XGZnhwPXQjuPV/wi7jjBfgL5ksDefqk2wKMkUdXLJ0wbcRZhPHUwp5yIL1kcImtr
uzKNUNgBPsmYSGFy1YXyE/0/kevz3Hjt8CQtYiGEL+ztWOv4kRowPNATd1B/BxpB
NKneaja9JMul48MBTKKtkf3/6032mn+sn/8BYDWns+3zddusoThtn0jYKFUAEeLz
XDXHnfL06BZzwjhiiLSVzfzlgGDKL1z6SZrKl8yT8iCwJ87XdAzb0NfGA3OFCVJ6
EsX+L1YEBad6MVRVvkSQV7AQEpZqOxMnXethQyg5wt4BxViqufOvLCff9cPBvU/G
c0XRzV0CAwEAAaNTMFEwHQYDVR0OBBYEFI8iwyJqJTTyFkdCo/YTQfD8Yv+oMB8G
A1UdIwQYMBaAFI8iwyJqJTTyFkdCo/YTQfD8Yv+oMA8GA1UdEwEB/wQFMAMBAf8w
DQYJKoZIhvcNAQELBQADggEBAE08v2gBDG2kYbNIRrsPuksIXEy0FgV9gALgsmHJ
syvf2JCCXIh/9s+LdXtA119mtVylVm5OCxhiq4dd/qYXnuD38tSxLqUzZKbhrT99
Q3V2bHtmBMn2IrXIJlnqmOGvqq03DdVALlSm4xxXSG1L8W2+sEOA54ZnuM1nkgah
lp2WhQMebLccf6TU0P50Lkg/yKBb7GYWRdC0wWSdz/jBIAafSWpJ612Wb24nzsQo
c/4jmoQYsTjGFh9qfE25sAnbdeheuYF/2Y1Tt42xY9D1f6po+IbzQMKEIxcYO4fJ
z1ZTTBPxlp02HMoR9VbEJXOMedm3BPxifCLKl3CpRWCOOTU=
-----END CERTIFICATE-----
)CERT";

const char prvtkey_pem[] = R"KEY(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCxGZ/2Jd+qX4TP
O1xmZ4cD10I7j1f8Iu44wX4C+ZLA3n6pNsCjJFHVyydMG3EWYTx1MKeciC9ZHCJr
a7syjVDYAT7JmEhhctWF8hP9P5Hr89x47fAkLWIhhC/s7Vjr+JEaMDzQE3dQfwca
QTSp3mo2vSTLpePDAUyirZH9/+tN9pp/rJ//AWA1p7Pt83XbrKE4bZ9I2ChVABHi
81w1x53y9OgWc8I4Yoi0lc385YBgyi9c+kmaypfMk/IgsCfO13QM29DXxgNzhQlS
ehLF/i9WBAWnejFUVb5EkFewEBKWajsTJ13rYUMoOcLeAcVYqrnzrywn3/XDwb1P
xnNF0c1dAgMBAAECggEANO/Bajyw6gbKNn16ZZu9dk7yG8aBmLrCNVHU/kOmc6Mt
/huBImK5WWvzr6KmP3fKMAdemoH0kTDBMxXNK1OlXiOlJnIv9Hz0SSjug6zsS7il
ZrpMpSuxoO4XuzzI7yPqD4dqrhGbgQKvV1A1Uy9J1VndNUg9V+08AIZcVK2aIr+7
SEC8cnQT3WBYk1IdnvbtLqk4kF4VpSn9dIj6o894j8oa5jMtH3RyvcqTENFTfcyA
gJLxzcdmiZjXN2kJzi0RvgsXFPQ2fo34ZVE0tEb01ZymFhP5hlxdTH+7s/sA8jKa
opqcBwiGutyy7NdnOd7vhqjcysQelTYW0nsO3fE8ewKBgQDrbkFT7lytHSCpFmXW
yMuPQhfEWDhUsYW/TemSELLZWY7OjQohKg5B7UTCMFTswoNL1IJy6do7AFwxAQ4q
opH7NZLO09tREjbwr99mf0hZ2dl84GEJwICrAuvqaH5KuQnQFEDQNl43fT74ateh
S1vd6e/qMPaB5DOgnanX814fmwKBgQDAkrjyJzSQQ1fBb4O6wE6WJW2QiOtjFCZ5
rOos4MdSH+vEAFHE1BpQ+1LxXrigb8K51NU+0Nymxug66oiCd03h68xGzKC33y29
baMpFaMSshlTzyrGmAJSZbd6SvA4HkDQi9GQcxT9yMu1ct3E7L8XPg+Hpi12LpiV
+XjsM4miZwKBgQCIncxO9gzVcAeZ1yN1hcOJ+CJuj3rwuqsQHhgh6IrAUZXrCjEM
maj4XMT/oqhFU4J5KHIiWM2IW2ckEKa12uy3R5lPZ+TmoJk9BfGvOsEzUpVAP7xP
umNv2RwpREW0+rdVBcuuhz4ZGRbHdMh4EpDWr3/G+G6zZUcINzVt2Q2PbQKBgBBs
MtOL3mKzK4e2j+BcSBdr57UcyB4x+LoFsg7TuIvbZwrXECIeCISBemhVMJ+WCGA6
s1jhDQA4FFP57lxZ+k1bA5fCGa60nLz73Z9CVh7hC2HnV/mJuyD1xbjrLSNMCTD/
y4ZrGg4ukpjNy/XgTseXplFmLZhXjvGzTcmu93zfAoGAHkTdkvuSW/T40PwuV1ji
UENCnS4Ku8H5O3XnExjRmtUZIoIPz2rHmRiqNvYPga+PsA1eusTfJPucR1JTV5hX
6qrS0oqfOyMAUxKBE/1a8Hug7+6aSYYYJMcMo2ACBlCXCeCDfyb6utl6Mv9SJ7Du
rAgKmPOje+4UchRdeY1QCt4=
-----END PRIVATE KEY-----
)KEY";

#endif
