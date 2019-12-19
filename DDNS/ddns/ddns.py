#!/usr/bin/python
# -*- coding: UTF-8 -*-

from aliyunsdkcore.client import AcsClient
from aliyunsdkcore.acs_exception.exceptions import ClientException
from aliyunsdkcore.acs_exception.exceptions import ServerException
import os
import time
import urllib2
client = AcsClient('LTAI4FgYQiqRDs69PucAMkbq', 'p67LkZib6cHG2RR5tNtzXME1TgGveV', 'cn-shenzhen')

def get_internet_ip():
      response=urllib2.urlopen('http://www.3322.org/dyndns/getip')
      html = response.read()
      ip = str(html).replace("\n", "")
      return ip

print get_internet_ip()

def Describe_SubDomain_Records(client,record_type,subdomain):
    request = DescribeSubDomainRecordsRequest()
    request.set_accept_format('json')

    request.set_Type(record_type)
    request.set_SubDomain(subdomain)

    response = client.do_action_with_exception(request)
    response = str(response, encoding='utf-8')
    relsult = json.loads(response)
    return relsult
    
des_relsult = Describe_SubDomain_Records(client,"A","sz.huangwx.cn")
