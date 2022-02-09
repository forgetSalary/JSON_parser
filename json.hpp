#ifndef K1804_JSON_CPP_H
#define K1804_JSON_CPP_H

#include "JSON.h"
#include "typeindex"
#include "map"
#include "iostream"

class JsonError : public std::exception{
public:
    virtual const char* what() const throw() = 0;
};

class JsonUnknownKeyError : public JsonError{
public:
    virtual const char* what() const throw(){
        return "UnknownKey";
    }
};

class JsonTypeMismatchError : public JsonError{
public:
    virtual const char* what() const throw(){
        return "TypeMismatch";
    }
};

enum{JSON_ARENA};

void* operator new[] (size_t size,int id){
    if (id == JSON_ARENA){
        return json_alloc(size);
    }
    char* bytes = new char[size];
    return bytes;
}

namespace JSON{

    class Object;
    class ObjectIterator;

    static std::map<std::type_index,JsonType> json_type_map = {
            {std::type_index(typeid(int)),JSON_number_int},
            {std::type_index(typeid(double)),JSON_number_float},
            {std::type_index(typeid(char*)),JSON_string},
            {std::type_index(typeid(const char*)),JSON_string},
            {std::type_index(typeid(JsonString)),JSON_string},
            {std::type_index(typeid(bool)),JSON_bool},
            {std::type_index(typeid(nullptr)),JSON_null},
            {std::type_index(typeid(JsonArray)),JSON_array},
            {std::type_index(typeid(JsonObject*)),JSON_object},
    };

    class Value{
        JsonValue* value;
    public:
        static void set(JsonValue* dst,int value){
            dst->int_number = value;
        }
        static void set(JsonValue* dst,double value){
            dst->float_number = value;
        }
        static void set(JsonValue* dst,char* value){
            dst->string = json_string(value);
        }
        static void set(JsonValue* dst,JsonString value){
            dst->string = value;
        }
        static void set(JsonValue* dst,bool value){
            dst->boolean = value;
        }
        static void set(JsonValue* dst,JsonArray value){
            dst->array = value;
        }
        static void set(JsonValue* dst,JsonObject* value){
            dst->object = value;
        }
        Value(JsonValue* val){
            this->value = val;
        }
        Value(int value){
            this->value = json_value_number_int(value);
        }
        Value(double value){
            this->value =  json_value_number_float(value);
        }
        Value(JsonString value){
            this->value =  json_value_string(value);
        }
        Value(const char* value){
            this->value = json_value_string(json_string(value));
        }
        Value(char* value){
            this->value = json_value_string(json_string(value));
        }
        Value(bool value){
            this->value =  json_value_boolean(value);
        }
        Value(JsonArray value){
            this->value =  json_value_array(value.values,value.len);
        }
        Value(JsonValue** values,size_t count){
            this->value =  json_value_array(values,count);
        }
        Value(JsonObject* value){
            this->value =  json_value_object(value);
        }
        void operator=(JsonValue* value){
            this->value = value;
        }

#ifdef JSON_GENERATE_EXCEPTIONS
#define operator_type(token,json_type,...)\
        if (this->value) {      \
            if (this->value->type == json_type){ \
                return this->value->token;\
            }                             \
            throw JsonTypeMismatchError();\
        }\
        throw JsonUnknownKeyError();\
        return __VA_ARGS__
#else
#define operator_type(token,...)\
        return this->value ? this->value->token : __VA_ARGS__
#endif

        operator int() const{
            operator_type(int_number,JSON_number_int,0);
        }
        operator double () const{
            operator_type(float_number,JSON_number_float,0);
        }
        operator char*() const{
            operator_type(string.str,JSON_string,nullptr);
        }
        operator std::string() const{
            operator_type(string.str,JSON_string, nullptr);
        }
        operator JsonString () const{
            operator_type(string,JSON_string, (JsonString){nullptr,0});
        }
        operator bool() const{
            operator_type(boolean,JSON_bool,0);
        }
        operator JsonObject*() const{
            operator_type(object,JSON_object, nullptr);
        }
        operator JsonArray() const{
            operator_type(array,JSON_array, (JsonArray){nullptr,0});
        }
#undef operator_type
        inline JsonValue* operator*(){return this->value;}
        inline operator JsonValue*(){return this->value;}
    };

    template<typename T>
    class Array{
        JsonArray array;
    public:
        Array(JsonArray array){
            this->array = array;
        }
        Array(){
            array = {nullptr,0};
        }
        Array(std::initializer_list<T> list){
            array = {nullptr,0};
            for (auto val:list){
                json_array_push(&array,Value(val));
            }
        }
        inline Value begin(){
            return Value(*this->array.values);
        }
        inline Value end(){
            return Value(*(this->array.values + this->array.len));
        }
        T operator[](size_t i){
            return Value(array.values[i]);
        }
        inline size_t length(){
            return array.len;
        }
        inline void push(T value){
            json_array_push(&this->array,Value(value));
        }
        operator JsonArray(){return this->array;}
    };

    class Field{
        friend class ObjectIterator;
        friend class Object;
    private:
        JsonField* field;
        Field(JsonField* value){
            this->field = value;
        }
    public:
        inline char* key(){
            return field->key.str;
        }
        inline size_t key_length(){
            return field->key.len;
        }

        template<typename T>
        inline operator T(){return Value(this->field->value);}

        template<typename T>
        void operator=(T value){
            if (this->field->value){
                if (this->field->value->type == json_type_map[std::type_index(typeid(T))]){
                    Value::set(field->value, value);
                    return;
                }
            }
            field->value = Value(value);
        }

    };

    class ObjectIterator{
        friend Object;
    private:
        JsonField** it;
        ObjectIterator(JsonField** it){
            this->it = it;
        }
    public:
        inline void operator++(){
            it++;
        }
        inline bool operator!=(ObjectIterator& other){
            return this->it != other.it;
        }
        inline Field operator*(){
            return Field(*this->it);
        }
    };

    class Object{
        friend std::ostream& operator<<(std::ostream& os, const Object& object);
    private:
        JsonObject* object;
    public:
        Object(std::initializer_list<JsonField*> list){
            JsonObject* obj = json_object(NULL,0);
            for (auto field:list){
                json_put_field(obj,field);
            }
            this->object = obj;
        }

        Object(std::initializer_list<std::pair<const char*,JsonValue*>> list){
            JsonObject* obj = json_object(NULL,0);
            for (auto pair:list){
                json_put_field(obj, json_field(pair.first,pair.second));
            }
            this->object = obj;
        }
        Object(std::initializer_list<std::pair<const char*,JsonValue*>> list,bool format){
            *this = Object(list);
            object->format_print = format;
        }
        Object(){
            this->object = json_object(NULL,0);
        }
        Object(JsonObject* obj){
            this->object = obj;
        }
        Object(char* str){
            object = json_parse(str);
        }
        Object(const char* str){
            object = json_parse(const_cast<char*>(str));
        }
        ~Object(){
            //json_free_object(this->object);
        }

        Field operator[](const char* key){
            JsonField* field = json_get_field(this->object,key);
            if (!field){
                field = json_field(key, nullptr);
                json_put_field(this->object, field);
            }
            return Field(field);
        }

        inline void operator=(JsonObject* right){
            this->object = right;
        }

        inline void format_print(bool format){
            this->object->format_print = format;
        }

        inline ObjectIterator begin(){
            return ObjectIterator(this->object->fields);
        }

        inline ObjectIterator end(){
            return ObjectIterator(this->object->fields + this->object->fields_count);
        }

        inline size_t size(){
            return this->object->fields_count;
        }

        operator JsonObject* (){return this->object;}
    };

    std::ostream& operator<<(std::ostream& os, const Object& object){
        char* buffer = json_stringify(object.object);
        os << buffer;
        free(buffer);
        return os;
    }

}

#endif //K1804_JSON_CPP_H
