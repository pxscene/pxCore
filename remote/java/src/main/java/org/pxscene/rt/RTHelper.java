package org.pxscene.rt;


import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.ServerSocket;
import java.util.List;
import org.apache.log4j.Logger;
import org.pxscene.rt.remote.RTRemoteMessageType;
import org.pxscene.rt.remote.RTRemoteProtocol;
import org.pxscene.rt.remote.messages.RTMessageCallMethodRequest;
import org.pxscene.rt.remote.messages.RTMessageCallMethodResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByIndexRequest;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageGetPropertyRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByIndexRequest;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyByNameResponse;
import org.pxscene.rt.remote.messages.RTMessageSetPropertyRequest;

/**
 * the remote helper methods
 */
public final class RTHelper {

  /**
   * the logger instance
   */
  private static final Logger logger = Logger.getLogger(RTHelper.class);

  /**
   * find a free tcp port to use
   *
   * @return the port
   * @throws RTException it will raise a error if cannot found a port
   */
  public static int findRandomOpenPortOnAllLocalInterfaces() throws RTException {
    try {
      ServerSocket socket = new ServerSocket(0);
      int port = socket.getLocalPort();
      socket.close();
      return port;
    } catch (IOException e) {
      logger.error("cannot get random rpc port, please check your network interface", e);
      throw new RTException(e);
    }
  }

  /**
   * first capitalize a string
   *
   * @param str the word string
   * @return the word string with first capitalize
   */
  public static String firstCapitalize(String str) {
    return str.substring(0, 1).toUpperCase() + str.substring(1);
  }

  /**
   * dump object with fields and methods
   *
   * @param name the object name
   * @param object the object value
   */
  public static void dumpObject(String name, Object object) {
    Class<?> clazz = object.getClass();
    for (Field field : clazz.getFields()) {
      logger.debug("object name=" + name + ",field = " + field.getName()
          + ", type =" + field.getType().getName());
    }
    for (Method method : clazz.getMethods()) {
      logger.debug("object name=" + name + ", method = " + method.getName() + ", return type ="
          + method.getReturnType().getName());
    }
  }


  /**
   * get value by property name
   * first find object field or method get*
   *
   * @param object the object value
   * @param getRequest the request message
   * @return the get response with value
   */
  public static RTMessageGetPropertyByNameResponse getProperty(
      Object object, RTMessageGetPropertyRequest getRequest) {

    Class<?> clazz = object.getClass();
    String propertyName = getRequest.getPropertyName();
    RTStatus rtStatus = new RTStatus(RTStatusCode.OK);
    RTMessageGetPropertyByNameResponse getResponse = new RTMessageGetPropertyByNameResponse();
    getResponse.setObjectId(getRequest.getObjectId());
    getResponse.setStatus(rtStatus);
    getResponse.setCorrelationKey(getRequest.getCorrelationKey());

    Field field = null;
    try {
      field = clazz.getField(propertyName);
    } catch (NoSuchFieldException ignored) {
    }
    Method method = getMethodByName(clazz, "get" + firstCapitalize(propertyName));
    Method functionMethod = RTHelper.getMethodByName(clazz, propertyName);

    // get property by index
    if (getRequest.getMessageType() == RTRemoteMessageType.GET_PROPERTY_BYINDEX_REQUEST) {
      List<RTValue> arr = null;
      try {
        if (field != null) {
          arr = (List<RTValue>) field.get(object);
        }
        if (method != null) {
          arr = (List<RTValue>) method.invoke(object);
        }
        int index = ((RTMessageGetPropertyByIndexRequest) getRequest).getIndex();
        if (index >= arr.size()) {
          rtStatus.setCode(RTStatusCode.INVALID_ARGUMENT);
        } else {
          getResponse.setValue(arr.get(index));
          rtStatus.setCode(RTStatusCode.OK);
        }
      } catch (Exception e) {
        rtStatus.setCode(RTStatusCode.TYPE_MISMATCH);
      }
      return getResponse;
    }

    if (field == null && method == null && functionMethod == null) { // field or method not exist
      logger.error("cannot found property name = " + propertyName);
      rtStatus.setCode(RTStatusCode.PROPERTY_NOT_FOUND);
    }

    try {
      if (field != null) {
        Object value = field.get(object);
        getResponse.setValue(new RTValue(value));
      } else if (method != null) {
        Object value = method.invoke(object);
        if (value == null) {
          getResponse.setValue(null);
        } else if (value instanceof RTObject) {
          getResponse.setValue(new RTValue(value, RTValueType.OBJECT));
        } else if (value.getClass() == RTValue.class) {
          getResponse.setValue((RTValue) value);
        } else {
          getResponse.setValue(new RTValue(value));
        }
      } else if (functionMethod != null) {
        RTFunction function = new RTFunction();
        function.setFunctionName(propertyName);
        function.setObjectId(getRequest.getObjectId());
        RTValue rtValue = new RTValue();
        rtValue.setType(RTValueType.FUNCTION);
        rtValue.setValue(function);
        getResponse.setValue(rtValue);
      }
    } catch (IllegalAccessException e) {
      logger.error("set property failed", e);
      rtStatus.setCode(RTStatusCode.TYPE_MISMATCH);
    } catch (InvocationTargetException e) {
      logger.error("invoke failed", e);
      rtStatus.setCode(RTStatusCode.TYPE_MISMATCH);
    }
    return getResponse;
  }


  /**
   * get method by name
   *
   * @param clazz the class
   * @param name the method name
   * @return the Method
   */
  private static Method getMethodByName(Class<?> clazz, String name) {
    Method[] methods = clazz.getMethods();
    for (Method method : methods) {
      if (method.getName().equals(name)) {
        return method;
      }
    }
    return null;
  }

  /**
   * invoke object method by method name
   *
   * @param object the object value
   * @param request the invoke request message
   * @return the response with value
   */
  public static RTMessageCallMethodResponse invokeMethod(Object object,
      RTMessageCallMethodRequest request) {
    RTMessageCallMethodResponse response = new RTMessageCallMethodResponse();
    response.setCorrelationKey(request.getCorrelationKey());
    response.setStatus(new RTStatus(RTStatusCode.OK));

    Class<?> clazz = object.getClass();
    List<RTValue> args = request.getFunctionArgs();
    Method destMethod = null;
    int methodParamsCount = args == null ? 0 : args.size();

    // because of java method need signature to found and rtRemote call request didn't have signatures
    // so i need for each methods to find first method by the method name and count, and ignore java overloading
    // TODO note that method overloading ignored for now, maybe remote need update protocol
    Method[] methods = clazz.getMethods();
    for (Method method : methods) {
      if (method.getName().equals(request.getMethodName())
          && methodParamsCount == method.getParameterCount()) {
        destMethod = method;
        break;
      }
    }

    if (destMethod == null) {
      response.setStatus(new RTStatus(RTStatusCode.PROPERTY_NOT_FOUND));
    } else {
      try {
        Object[] callArgs = new Object[methodParamsCount];
        if (args != null && args.size() > 0) {
          for (int i = 0; i < methodParamsCount; i += 1) {
            callArgs[i] = args.get(i).getValue();
          }
        }

        Object returnValue = destMethod.invoke(object, callArgs);  // invoke method
        if (returnValue != null) {
          response.setValue(new RTValue(returnValue)); // set value
        }
      } catch (Exception e) {
        logger.error("invoke method " + request.getMethodName() + " failed", e);
        response.setStatus(new RTStatus(RTStatusCode.INVALID_ARGUMENT));
      }
    }
    return response;
  }

  /**
   * set object value by property name
   *
   * @param object the object value
   * @param request the set request message
   * @return the response
   */
  public static RTMessageSetPropertyByNameResponse setProperty(
      Object object, RTMessageSetPropertyRequest request) {

    RTMessageSetPropertyByNameResponse setResponse = new RTMessageSetPropertyByNameResponse();
    setResponse.setCorrelationKey(request.getCorrelationKey());
    RTStatusCode code = RTStatusCode.UNKNOWN;
    setResponse.setObjectId(request.getObjectId());

    Class<?> clazz = object.getClass();
    String propertyName = request.getPropertyName();

    Field field = null;
    try {
      field = clazz.getField(propertyName);
    } catch (NoSuchFieldException ignored) {
    }

    Method method;
    int index = -1;
    if (request.getMessageType()
        == RTRemoteMessageType.SET_PROPERTY_BYINDEX_REQUEST) {
      // set array index, so need get the list first
      index = ((RTMessageSetPropertyByIndexRequest) request).getIndex();
      method = getMethodByName(clazz, "get" + firstCapitalize(propertyName));
    } else {
      method = getMethodByName(clazz, "set" + firstCapitalize(propertyName));
    }

    if (field == null && method == null) {
      logger.error("cannot found property name = " + propertyName);
      code = RTStatusCode.PROPERTY_NOT_FOUND;
    } else {
      try {
        if (field != null) {
          if (index < 0) {
            code = setField(field, object, request.getValue());
          } else {
            code = setPropertyByIndex(field.get(object), index, request.getValue());
          }
        } else {
          if (index < 0) {
            code = setFieldByMethod(method, object, request.getValue());
          } else {
            code = setPropertyByIndex(method.invoke(object), index, request.getValue());
          }
        }
      } catch (Exception e) {
        logger.error("set property failed", e);
        setResponse.setStatusCode(RTStatusCode.TYPE_MISMATCH);
      }
    }
    setResponse.setStatusCode(code);
    return setResponse;
  }

  private static RTStatusCode setPropertyByIndex(Object listObj, int index, RTValue value) {
    List<RTValue> arr = (List<RTValue>) listObj;
    if (arr == null) {
      return RTStatusCode.TYPE_MISMATCH;
    }
    if (index >= arr.size()) {
      return RTStatusCode.INVALID_ARGUMENT;
    }

    RTValue v = arr.get(index);
    v.setType(value.getType());
    v.setValue(value.getValue());
    return RTStatusCode.OK;
  }

  /**
   * set value to field
   *
   * @param field the field in object
   * @param obj the object
   * @param value the rt value
   */
  private static RTStatusCode setField(Field field, Object obj, RTValue value)
      throws IllegalAccessException {
    if (field.getType() == RTValue.class) {
      field.set(obj, value);
    } else {
      field.set(obj, value.getValue());
    }
    return RTStatusCode.OK;
  }

  /**
   * set value to field by set* method
   *
   * @param method the setter method
   * @param obj the object
   * @param value the rt value
   */
  private static RTStatusCode setFieldByMethod(Method method, Object obj, RTValue value)
      throws InvocationTargetException, IllegalAccessException {
    Class<?>[] inputTypes = method.getParameterTypes();
    if (inputTypes.length > 0 && inputTypes[0] == RTValue.class) {
      method.invoke(obj, value);
    } else {
      method.invoke(obj, value.getValue());
    }
    return RTStatusCode.OK;
  }


  /**
   * create new listener if function listener is null
   *
   * @param protocol the protocol that send call request
   * @param oldFunction the old function
   * @return the newFunction
   */
  public static RTFunction updateListenerForRTFuction(RTRemoteProtocol protocol,
      RTFunction oldFunction) {

    if (oldFunction != null && oldFunction.getListener() != null) {
      return oldFunction;
    }

    RTFunction rtFunction = new RTFunction(rtValueList -> {
      RTValue[] args;
      if (rtValueList == null || rtValueList.size() == 0) {
        args = new RTValue[0];
      } else {
        args = rtValueList.toArray(new RTValue[rtValueList.size()]);
      }
      try {
        protocol
            .sendCallByNameAndNoReturns(oldFunction.getObjectId(),
                oldFunction.getFunctionName(),
                args).get();  // send call request

      } catch (Exception e) {
        logger.error("invoke remote function failed", e);
        throw new RTException(e);
      }
    });
    rtFunction.setFunctionName(oldFunction.getFunctionName());
    rtFunction.setObjectId(oldFunction.getObjectId());
    RTEnvironment.getRtFunctionMap().put(rtFunction.getFunctionName(), rtFunction);
    return rtFunction;
  }

  /**
   * check the value is null or not
   *
   * @param value the value
   * @param fieldName the value field name
   * @throws NullPointerException raise null exception if value is null
   */
  public static void ensureNotNull(Object value, String fieldName) {
    if (value == null) {
      throw new NullPointerException(fieldName + "shoud not be null");
    }
  }
}
