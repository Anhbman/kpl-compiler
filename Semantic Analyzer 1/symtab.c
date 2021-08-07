/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

Type* makeIntType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_INT;
  return type;
}

Type* makeCharType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_CHAR;
  return type;
}

Type* makeArrayType(int arraySize, Type* elementType) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_ARRAY;
  type->arraySize = arraySize;
  type->elementType = elementType;
  return type;
}

Type* duplicateType(Type* type) {
  Type * newType = (Type*)malloc(sizeof(Type));
  newType->typeClass = type->typeClass;
  newType->arraySize = type->arraySize;
  newType->elementType = type->elementType;

  return newType;
}

int compareType(Type* type1, Type* type2) {
    if (type1->typeClass == type2->typeClass) {
        if (type1->typeClass == TP_ARRAY) {
            if (!(type1->arraySize == type2->arraySize
                    && compareType(type1->elementType, type2->elementType)))
                return 0;
        }
        return 1;
    }
    return 0;
}

void freeType(Type* type) {
  if (type->elementType != NULL)
  {
    freeType(type->elementType);
  }
  free(type);
  type = NULL;
  
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i) {
  ConstantValue * constValue = (ConstantValue*)malloc(sizeof(ConstantValue));
  constValue->type = TP_INT;
  constValue->intValue = i;

  return constValue;
}

ConstantValue* makeCharConstant(char ch) {
  ConstantValue * constValue = (ConstantValue*)malloc(sizeof(ConstantValue));
  constValue->type = TP_CHAR;
  constValue->charValue = ch;

  return constValue;
}

ConstantValue* duplicateConstantValue(ConstantValue* v) {
  ConstantValue * constValue = (ConstantValue*)malloc(sizeof(ConstantValue));
  constValue->type = v->type;
  if(v->type == TP_INT)
    constValue->intValue = v->intValue;
  else if (v->type == TP_CHAR)
  {
    constValue->charValue = v->charValue;
  }

  return constValue;
  
}

/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer) {
  Scope* scope = (Scope*) malloc(sizeof(Scope));
  scope->objList = NULL;
  scope->owner = owner;
  scope->outer = outer;
  return scope;
}

Object* createProgramObject(char *programName) {
  Object* program = (Object*) malloc(sizeof(Object));
  strcpy(program->name, programName);
  program->kind = OBJ_PROGRAM;
  program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
  program->progAttrs->scope = createScope(program,NULL);
  symtab->program = program;

  return program;
}

Object* createConstantObject(char *name) {
  Object* constantObject = (Object*)malloc(sizeof(Object));
  strcpy(constantObject->name, name);
  constantObject->kind = OBJ_CONSTANT;
  constantObject->constAttrs = (ConstantAttributes*)malloc(sizeof(ConstantAttributes));
  constantObject->constAttrs->value = NULL;

  return constantObject;
}

Object* createTypeObject(char *name) {
  Object* typeObject = (Object*)malloc(sizeof(Object));
  strcpy(typeObject->name,name);
  typeObject->kind = OBJ_TYPE;
  typeObject->typeAttrs = (TypeAttributes*)malloc(sizeof(TypeAttributes));
  typeObject->typeAttrs->actualType = NULL;

  return typeObject;
}

Object* createVariableObject(char *name) {
  Object *variableObject = (Object*)malloc(sizeof(Object));
  variableObject->kind = OBJ_VARIABLE;
  strcpy(variableObject->name,name);
  variableObject->varAttrs = (VariableAttributes*)malloc(sizeof(VariableAttributes));
  variableObject->varAttrs->type = NULL;
  variableObject->varAttrs->scope = symtab->currentScope;

  return variableObject;
}

Object* createFunctionObject(char *name) {
  Object *functionObject = (Object*)malloc(sizeof(Object));
  strcpy(functionObject->name,name);
  functionObject->kind = OBJ_FUNCTION;
  functionObject->funcAttrs = (FunctionAttributes*)malloc(sizeof(FunctionAttributes));
  functionObject->funcAttrs->paramList = NULL;
  functionObject->funcAttrs->returnType = NULL;
  functionObject->funcAttrs->scope = createScope(functionObject, symtab->currentScope);

  return functionObject;

}

Object* createProcedureObject(char *name) {
  Object *procedureObject = (Object*)malloc(sizeof(Object));
  strcpy(procedureObject->name,name);
  procedureObject->kind = OBJ_PROCEDURE;
  procedureObject->procAttrs = (ProcedureAttributes*)malloc(sizeof(ProcedureAttributes));
  procedureObject->procAttrs->paramList = NULL;
  procedureObject->procAttrs->scope = createScope(procedureObject, symtab->currentScope);

  return procedureObject;
}

Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
  Object *parameterObject = (Object*)malloc(sizeof(Object));
  strcpy(parameterObject->name, name);
  parameterObject->kind = OBJ_PARAMETER;
  parameterObject->paramAttrs = (ParameterAttributes*)malloc(sizeof(ParameterAttributes));
  parameterObject->paramAttrs->kind = kind;
  parameterObject->paramAttrs->type = NULL;
  parameterObject->paramAttrs->function = owner;

  return parameterObject;
}

void freeObject(Object* obj) {
  if (obj != NULL) {
        if (obj->constAttrs != NULL) {
            switch (obj->kind) {
            case OBJ_CONSTANT:
                if (obj->constAttrs->value != NULL) {
                    free(obj->constAttrs->value);
                    obj->constAttrs->value = NULL;
                }
                break;
            case OBJ_VARIABLE:
                if (obj->varAttrs->type != NULL) {
                    free(obj->varAttrs->type);
                    obj->varAttrs->type = NULL;
                }
                break;
            case OBJ_TYPE:
                if (obj->typeAttrs->actualType != NULL) {
                    free(obj->typeAttrs->actualType);
                    obj->typeAttrs->actualType = NULL;
                }
                break;
            case OBJ_PROGRAM:
                if (obj->progAttrs->scope != NULL) {
                    freeScope(obj->progAttrs->scope);
                    obj->progAttrs->scope = NULL;
                }
                break;
            case OBJ_FUNCTION:
                freeScope(obj->funcAttrs->scope); // Free scope also free the param list
                break;
            case OBJ_PROCEDURE:
                freeScope(obj->procAttrs->scope); // Free scope also free the param list
                break;
            case OBJ_PARAMETER:
                if (obj->paramAttrs->type != NULL) {
                    free(obj->paramAttrs->type);
                    obj->paramAttrs->type = NULL;
                }
                break;
            default:
                break;
            }

            free(obj->constAttrs);
            obj->constAttrs = NULL;
        }
        free(obj);
        obj = NULL;
    }
}

void freeScope(Scope* scope) {
  if (scope != NULL)
  {
    freeObjectList(scope->objList);
    free(scope);
    scope = NULL;
  }
  
}

void freeObjectList(ObjectNode *objList) {
  if (objList != NULL)
  {
    freeObject(objList->object);
    freeObjectList(objList->next);
    objList = NULL;
  }
  
}

void freeReferenceList(ObjectNode *objList) {
  if (objList != NULL) 
  {
    freeObject(objList->object);
    freeReferenceList(objList->next);
    objList = NULL;
  }
}

void addObject(ObjectNode **objList, Object* obj) {
  ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
  node->object = obj;
  node->next = NULL;
  if ((*objList) == NULL) 
    *objList = node;
  else {
    ObjectNode *n = *objList;
    while (n->next != NULL) 
      n = n->next;
    n->next = node;
  }
}

Object* findObject(ObjectNode *objList, char *name) {
  ObjectNode *currentNode = objList;
  while (currentNode != NULL)
  {
    if(strcmp(currentNode->object->name, name) == 0){
      return currentNode->object;
    }
    currentNode = currentNode->next;
  }
  
  return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
  Object* obj;
  Object* param;

  symtab = (SymTab*) malloc(sizeof(SymTab));
  symtab->globalObjectList = NULL;
  
  obj = createFunctionObject("READC");
  obj->funcAttrs->returnType = makeCharType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createFunctionObject("READI");
  obj->funcAttrs->returnType = makeIntType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEI");
  param = createParameterObject("i", PARAM_VALUE, obj);
  param->paramAttrs->type = makeIntType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEC");
  param = createParameterObject("ch", PARAM_VALUE, obj);
  param->paramAttrs->type = makeCharType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITELN");
  addObject(&(symtab->globalObjectList), obj);

  intType = makeIntType();
  charType = makeCharType();
}

void cleanSymTab(void) {
  freeObject(symtab->program);
  freeObjectList(symtab->globalObjectList);
  free(symtab);
  freeType(intType);
  freeType(charType);
}

void enterBlock(Scope* scope) {
  symtab->currentScope = scope;
}

void exitBlock(void) {
  symtab->currentScope = symtab->currentScope->outer;
}

void declareObject(Object* obj) {
  if (obj->kind == OBJ_PARAMETER) {
    Object* owner = symtab->currentScope->owner;
    switch (owner->kind) {
    case OBJ_FUNCTION:
      addObject(&(owner->funcAttrs->paramList), obj);
      break;
    case OBJ_PROCEDURE:
      addObject(&(owner->procAttrs->paramList), obj);
      break;
    default:
      break;
    }
  }
 
  addObject(&(symtab->currentScope->objList), obj);
}


