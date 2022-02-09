typedef enum JsonType{
    JSON_number_int,    //int
    JSON_number_float,  //double
    JSON_string,        //const char*
    JSON_bool,          //std bool
    JSON_array,         //array of JSON values
    JSON_object,        //JSON object
    JSON_null           //NULL
}JsonType;

typedef struct JsonObject JsonObject;
typedef struct JsonValue JsonValue;

typedef struct JsonString{
    char* str;
    size_t len;
}JsonString;

typedef struct JsonArray{
    BUF(JsonValue** values);
    size_t len;
}JsonArray;

struct JsonValue{
    JsonType type;
    union{
        double float_number;
        int int_number;
        JsonString string;
        bool boolean;
        JsonArray array;
        JsonObject* object;
    };
};

typedef struct JsonField{
    JsonString key;
    JsonValue* value;
    struct JsonField* next;//for hash map
}JsonField;

struct JsonObject{
    bool format_print;
    BUF(JsonField** fields);
    size_t fields_count;
    Map* fields_map;
};

void free_json_data();

void json_put_field(JsonObject* object,JsonField* field);

JsonField* json_get_field(JsonObject* obj,const char* key);

inline JsonString json_string(const char* str){
    return (JsonString){str,strlen(str)};
}

void json_array_push(JsonArray* array, JsonValue* value);

JsonValue* json_value_number_float(double val);

JsonValue* json_value_number_int(int val);

JsonValue* json_value_string(JsonString val);

JsonValue* json_value_boolean(bool value);

JsonValue* json_value_array(JsonValue** values, size_t count);

JsonValue* json_value_object(JsonObject* value);

JsonValue* json_value_null();

JsonField* json_field(const char* key,JsonValue* value);

JsonObject* json_object(JsonField** fields,size_t fields_count);

JsonObject* json_parse(char* str);

void* json_alloc(size_t size);

char* json_stringify(JsonObject* object);

void json_free_object(JsonObject* obj);

void json_fprintf(FILE* stream,JsonObject* obj);
