// automatically generated by admSerialization.py, do not edit!
#include "ADM_default.h"
#include "ADM_paramList.h"
#include "ADM_coreJson.h"
#include "flat360.h"
bool  flat360_jserialize(const char *file, const flat360 *key){
admJson json;
json.addUint32("method",key->method);
json.addUint32("algo",key->algo);
json.addUint32("pad",key->pad);
json.addFloat("yaw",key->yaw);
json.addFloat("pitch",key->pitch);
json.addFloat("roll",key->roll);
json.addFloat("fov",key->fov);
json.addFloat("distortion",key->distortion);
return json.dumpToFile(file);
};
bool  flat360_jdeserialize(const char *file, const ADM_paramList *tmpl,flat360 *key){
admJsonToCouple json;
CONFcouple *c=json.readFromFile(file);
if(!c) {ADM_error("Cannot read json file");return false;}
bool r= ADM_paramLoadPartial(c,tmpl,key);
delete c;
return r;
};