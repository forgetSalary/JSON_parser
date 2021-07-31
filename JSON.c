#include "JSON.h"

JsonObject* json_object(JsonField** fields,size_t fields_count);
void json_put_field(JsonObject* object,JsonField* field);

Arena JSON_arena = {0};

void free_json_data(){
    arena_free(&JSON_arena);
}

inline JsonString json_string(const char* str){
    return (JsonString){str,strlen(str)};
}

void json_array_push(JsonArray* array, JsonValue* value){
    buf_push(array->values, value);
    array->len ++;
}

JsonValue* json_value_number(double val){
    JsonValue* value = arena_alloc(&JSON_arena, sizeof(JsonValue));
    value->type = JSON_number;
    value->number = val;
    return value;
}

JsonValue* json_value_string(JsonString val){
    JsonValue* value = arena_alloc(&JSON_arena, sizeof(JsonValue));
    value->type = JSON_string;
    value->string = val;
    return value;
}

JsonValue* json_value_boolean(bool value){
    JsonValue* val = arena_alloc(&JSON_arena, sizeof(JsonValue));
    val->type = JSON_bool;
    val->boolean = value;
    return val;
}

JsonValue* json_value_array(JsonValue** values, size_t count){
    JsonValue* val = arena_alloc(&JSON_arena, sizeof(JsonValue));
    val->type = JSON_array;
    val->array.values = NULL;
    val->array.len = count;
    for (JsonValue** value = values; value != values + count;value++){
        json_array_push(&val->array,*value);
    }
    return val;
}

JsonValue* json_value_object(JsonObject* value){
    JsonValue* val = arena_alloc(&JSON_arena, sizeof(JsonValue));
    val->type = JSON_object;
    if(value){
        val->object = value;
    }else{
        val->object = json_object(NULL,0);
    }
    return val;
}

JsonValue* json_value_null(){
    JsonValue* val = arena_alloc(&JSON_arena, sizeof(JsonValue));
    val->type = JSON_null;
    return val;
}

JsonField* json_field(const char* key,JsonValue* value){
    JsonField* f = arena_alloc(&JSON_arena,sizeof(JsonField));
    f->key.str = key;
    f->key.len = strlen(key);
    f->value = value;
    return f;
}

JsonObject* json_object(JsonField** fields,size_t fields_count){
    JsonObject* obj = arena_calloc(&JSON_arena,1,sizeof(JsonObject));
    obj->fields_map = arena_calloc(&JSON_arena,1,sizeof(Map));
    for (JsonField** field = fields; field && field != fields+fields_count; field++){
        json_put_field(obj,*field);
    }
    return obj;
}

void json_put_field(JsonObject* object,JsonField* field){
    uint64_t hash = str_hash(field->key.str,field->key.len);
    JsonField* hashed = map_get_hashed(object->fields_map,(void*)hash,hash);
    field->next = hashed;
    map_put_hashed(object->fields_map,(void*)hash,field,hash);
    buf_push(object->fields,field);
    object->fields_count ++;
}

JsonField* json_get_field(JsonObject* obj,const char* key){
    uint64_t hash = str_hash(key,strlen(key));
    JsonField* hashed = map_get_hashed(obj->fields_map,(void*)hash,hash);
    if(!hashed) return NULL;
    for (JsonField* field = hashed; field; field = field->next){
        if(!strcmp(field->key.str,key)){
            return field;
        }
    }
    return NULL;
}
