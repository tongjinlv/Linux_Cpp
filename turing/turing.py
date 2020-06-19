import json
import urllib.request
import base64
import json
import time
import hashlib
import urllib.request
import urllib.parse
import os
def chat(question):
    api_url = "http://openapi.tuling123.com/openapi/api/v2"
    text_input = question['text']
    req = {
        "perception":
        {
            "inputText":
            {
                "text": text_input
            },

            "selfInfo":
            {
                "location":
                {
                    "city": "深圳",
                    "province": "深圳",
                    "street": "西丽"
                }
            }
        },

        "userInfo": 
        {
            "apiKey": "98207818ccae432bb66142053ef73afb",
            "userId": "57a2cf6953813542"
        }
    }
    # 将字典格式的req转为utf8编码的字符串
    req = json.dumps(req).encode('utf8')
    
    print('\n' + '正在调用图灵机器人API...')
    http_post = urllib.request.Request(api_url, data=req, headers={'content-type': 'application/json'})
    response = urllib.request.urlopen(http_post)
    
    print('得到回答，输出为字典格式：')
    response_str = response.read().decode('utf8')
    response_dic = json.loads(response_str)
    intent_code = response_dic['intent']['code']
    
    # 返回网页类的输出方式
    if(intent_code == 10023):
        results_url = response_dic['results'][0]['values']['url']
        results_text = response_dic['results'][1]['values']['text']
        answer = {"code": intent_code, "text": results_text, "url":results_url}
        print(answer)
        return(answer)
    # 一般的输出方式
    else:
        results_text = response_dic['results'][0]['values']['text']
        answer = {"code": intent_code, "text": results_text}
        print(answer)
        return(answer)
    
def turing(question):
    eg_question = {'text': question, 'confidence': 0.9}
    return chat(eg_question)




