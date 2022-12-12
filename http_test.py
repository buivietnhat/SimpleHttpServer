import requests

url = 'http://0.0.0.0:8080/index.html'
r = requests.get(url, verify = False)
print(r.text)