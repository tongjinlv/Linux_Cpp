import requests
import datetime
import socket
def get_host_ip():
    """
    查询本机ip地址
    :return: ip
    """
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    finally:
        s.close()
    return ip
 
hostname = socket.gethostname()
ip = socket.gethostbyname(hostname)
ip="0.0.0.0"
ip=get_host_ip()
date=datetime.datetime.now()
response = requests.get('http://dm.trtos.com/web/datamark/update.php?id=5oOeooHD&index=149653&name=树莓派IP&value='+ip+":"+str(date))
response.text