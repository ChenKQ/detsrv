import requests
import base64


serverip = "172.20.10.3"
port = "8080"
router = "detectionMinic"
url = "http://" + serverip + ":" + port + "/" + router

def request(imgFile):
    with open(imgFile, 'rb') as f:
        rdata = f.read()
    e64data = base64.b64encode(rdata)
    prm = {'img': e64data}
    ret = requests.post(url, prm)
    print(ret.text)

def request2():
    prm = {'img': "hello world", "size": "128"}
    ret = requests.post(url, prm)
    print(ret.text)

def request3(imgFile):
    with open(imgFile, 'rb') as f:
        rdata = f.read()
    prm = {'img': rdata}
    ret = requests.post(url, prm)
    print(ret.text)


if __name__ == '__main__':
    request("test.jpg")
    # request2()
    # request3('test.jpg')
    # main()
    # imgFile = '../test.jpg'
    # img = cv2.imread(imgFile)
    # img2 = cv2.resize(img, (10,10))
    # cv2.imwrite('../test5.jpg', img2)