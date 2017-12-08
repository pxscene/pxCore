#include <sstream>

#include "rtPermissions.h"
#include "test_includes.h" // Needs to be included last

class rtPermissions2Test : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void test()
  {
    // Bootstrap -> FullTrust
    // Allow everything
    EXPECT_TRUE (allows(NULL, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://example.com", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://github.com", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://localhost", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://example.com", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://example.com", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://github.com", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://github.com:80", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://github.com:80/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:10004", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[::1]:50050/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://::1:50050/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[::1]:1001/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://::1:1001/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost/", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost:10004", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost:50050", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("serviceManager://", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("serviceManager://org.rdk.soundPlayer_1", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("serviceManager://com.comcast.application", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("serviceManager://com.comcast.stateObserver", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("serviceManager://com.comcast.FrameRate", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("feature://", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("feature://screenshot", "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("feature://unknown", "https://xfinity.comcast.com"));

    // Bootstrap -> LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://example.com", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://github.com", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://localhost", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://example.com", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://example.com", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://github.com", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://github.com:80", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://github.com:80/", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("http://localhost", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("http://localhost/", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("http://localhost:10004", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[::1]:50050/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://::1:50050/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[::1]:1001/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://::1:1001/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost/", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost:10004", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost:50050", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("serviceManager://", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("serviceManager://org.rdk.soundPlayer_1", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("serviceManager://com.comcast.FrameRate", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("feature://", "https://foo.partner2.com"));
    EXPECT_TRUE (allows("feature://screenshot", "https://foo.partner2.com"));
    EXPECT_FALSE(allows("feature://unknown", "https://foo.partner2.com"));

    // Bootstrap -> Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    EXPECT_TRUE (allows(NULL, "http://example.com"));
    EXPECT_TRUE (allows("", "http://example.com"));
    EXPECT_TRUE (allows("http://example.com", "http://example.com"));
    EXPECT_TRUE (allows("foo://", "http://example.com"));
    EXPECT_TRUE (allows("foo://example.com", "http://example.com"));
    EXPECT_TRUE (allows("foo://github.com", "http://example.com"));
    EXPECT_TRUE (allows("foo://localhost", "http://example.com"));
    EXPECT_TRUE (allows("http://", "http://example.com"));
    EXPECT_TRUE (allows("https://", "http://example.com"));
    EXPECT_TRUE (allows("http://example.com", "http://example.com"));
    EXPECT_TRUE (allows("https://example.com", "http://example.com"));
    EXPECT_TRUE (allows("https://github.com", "http://example.com"));
    EXPECT_TRUE (allows("https://github.com:80", "http://example.com"));
    EXPECT_TRUE (allows("https://github.com:80/", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost/", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:10004", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getDeviceId", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getAuthToken", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getSessionToken", "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getServiceAccessToken", "http://example.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", "http://example.com"));
    EXPECT_TRUE (allows("http://[::1]:50050/", "http://example.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", "http://example.com"));
    EXPECT_TRUE (allows("http://::1:50050/", "http://example.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", "http://example.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", "http://example.com"));
    EXPECT_TRUE (allows("http://[::1]:1001/", "http://example.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", "http://example.com"));
    EXPECT_TRUE (allows("http://::1:1001/", "http://example.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", "http://example.com"));
    EXPECT_TRUE (allows("https://localhost", "http://example.com"));
    EXPECT_TRUE (allows("https://localhost/", "http://example.com"));
    EXPECT_TRUE (allows("https://localhost:10004", "http://example.com"));
    EXPECT_TRUE (allows("https://localhost:50050", "http://example.com"));
    EXPECT_FALSE(allows("serviceManager://", "http://example.com"));
    EXPECT_FALSE(allows("serviceManager://org.rdk.soundPlayer_1", "http://example.com"));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", "http://example.com"));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", "http://example.com"));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", "http://example.com"));
    EXPECT_TRUE (allows("feature://", "http://example.com"));
    EXPECT_FALSE(allows("feature://screenshot", "http://example.com"));
    EXPECT_TRUE (allows("feature://unknown", "http://example.com"));
  }

private:
  bool allows(const char* url, const char* origin)
  {
    rtPermissions* p = new rtPermissions();
    EXPECT_TRUE (RT_OK == p->clearBootstrapConfig());
    EXPECT_TRUE (RT_OK == p->loadBootstrapConfig("supportfiles/pxscenepermissions.sample.conf"));
    EXPECT_TRUE (RT_OK == p->setOrigin(origin));

    bool allows;
    EXPECT_TRUE (RT_OK == p->allows(url, allows));
    EXPECT_TRUE (RT_OK == p->clearBootstrapConfig());
    delete p;
    return allows;
  }
};

TEST_F(rtPermissions2Test, rtPermissionsTests)
{
  test();
}
