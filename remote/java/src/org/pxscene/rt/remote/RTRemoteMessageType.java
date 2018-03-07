package org.pxscene.rt.remote;

public enum RTRemoteMessageType {
  SESSION_OPEN_REQUEST("session.open.request"),
  SESSION_OPEN_RESPIONSE("session.open.response"),
  GET_PROPERTY_BYNAME_REQUEST("get.byname.request"),
  GET_PROPERTY_BYNAME_RESPONSE("get.byname.response"),
  SET_PROPERTY_BYNAME_REQUEST("set.byname.request"),
  SET_PROPERTY_BYNAME_RESPONSE("set.byname.response"),
  KEEP_ALIVE_REQUEST("keeep_alive.request"),
  KEEP_ALIVE_RESPONSE("keep_alive.response"),
  SERACH_OBJECT("search"),
  LOCATE_OBJECT("locate");

  private final String m_stringRep;

  RTRemoteMessageType(String s) {
    m_stringRep = s;
  }

  public String toString() {
    return m_stringRep;
  }
}
