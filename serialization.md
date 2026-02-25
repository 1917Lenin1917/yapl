```
Module

  1 2 3 4 5 6 7 8     
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|    version      |          md5 (32 bytes)           |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|                     code_object                     |  <- Embedded CodeObject
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +



CodeObject

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|             name_len              |      name       |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|            names_len              |      names      |  <- Each len + data
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|            locals_len             |      locals     |  <- Each len + data
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|            params_len             |      params     |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - + 
|            consts_len             |      consts     |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - + 
|            opcode_len             |      opcode     |  <- 4 bytes per opcode
+ - - - - - - - - + - - - - - - - - + - - - - - - - - + 



Parameter

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|             name_len              |      name       |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|       kind      |   has_default   | 
+ - - - - - - - - + - - - - - - - - + 
|       default_const_index         |
+ - - - - - - - - + - - - - - - - - + 
|            local_index            |
+ - - - - - - - - + - - - - - - - - + 

BooleanValue

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + - - - - - - - - +
|      type       |      value      |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|         module_name_len           |   module_name   |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +



IntegerValue

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + 
|      type       |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|         module_name_len           |   module_name   |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|            value_len              |      value      |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +



FloatValue

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + 
|      type       |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|         module_name_len           |   module_name   |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|            value_len              |      value      |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +



StringValue

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + 
|      type       |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|         module_name_len           |   module_name   |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +
|            value_len              |      value      |
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +



CodeObjectValue

  1 2 3 4 5 6 7 8   1 2 3 4 5 6 7 8  
+ - - - - - - - - + 
|      type       |                                        
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +    
|         module_name_len           |   module_name   |    
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +    
|                    code_object                      |  <- Embedded CodeObject
+ - - - - - - - - + - - - - - - - - + - - - - - - - - +

```