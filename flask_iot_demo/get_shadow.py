# coding: utf-8

Project_ID = "91013bc50fd941e68ffb86f478b50c18"
Instance_ID = "368805c9-c87f-4a14-80c2-57398b92cdb0"
Device_ID = "67e53e652902516e866b8487_potato-sensor-test"
App_ID = "c4ac0b5acc504d668506981d54c3f435"
EndPoint = "https://9b0766b4b1.st1.iotda-app.cn-north-4.myhuaweicloud.com"

import os
from huaweicloudsdkcore.auth.credentials import BasicCredentials
from huaweicloudsdkcore.auth.credentials import DerivedCredentials
from huaweicloudsdkcore.region.region import Region as coreRegion
from huaweicloudsdkcore.exceptions import exceptions
from huaweicloudsdkiotda.v5 import *


ak = os.environ["HUAWEICLOUD_SDK_AK"]
sk = os.environ["HUAWEICLOUD_SDK_SK"]

iotdaEndpoint = EndPoint
projectId = Project_ID

credentials = BasicCredentials(ak, sk, projectId).with_derived_predicate(DerivedCredentials.get_default_derived_predicate())
client = IoTDAClient.new_builder() \
    .with_credentials(credentials) \
    .with_region(coreRegion(id="cn-north-4", endpoint=EndPoint)) \
    .build()

def get_shadow():
    try:
        request = ShowDeviceShadowRequest()
        request.device_id = Device_ID
        response = client.show_device_shadow(request)
        print("Successful get shadow!")
        print(response)
        return response.to_dict()
    except exceptions.ClientRequestException as e:
        print(e.status_code)
        print(e.request_id)
        print(e.error_code)
        print(e.error_msg)
        return None

if __name__ == "__main__":

    get_shadow()