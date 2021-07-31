typedef enum JsonType{
    JSON_number,        //double
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
        double number;
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
    BUF(JsonField** fields);
    size_t fields_count;
    Map* fields_map;
};