=====================================================================
JANET SOAP API

Module Documentation
=====================================================================

Author: Phillip Kynaston


--OVERVIEW AND REMIT-------------------------------------------------

This module is designed to provide an API Authentication layer for
 the SOAP Server module, which otherwise doesn't have any
 authentication


--API DEFINITION-----------------------------------------------------
The WSDL definition can be found at: /api/soap?wsdl


--EXAMPLE USAGE------------------------------------------------------

This is purely an example. The code returns the user object for user
 #97 (PK)

<code>
try {
  $params = new stdClass;
  // this is the API Key as provided by Janet
  $params->key = "SERVICE_KEY";
  // this is optional, a descriptive name for internal reporting
  // (internal reporting is not currently implemented)
  $params->name = "SERVICE_NAME";

  // create the client - this URL needs to be where the soap server
  // is defined
  $client = new SoapClient('https://community.ja.net/api/soap?wsdl');
  // add the authentication headers - keep as below
  $headers = array(new SoapHeader('https://community.ja.net', 'AuthHeader', $params));
  $client->__setSoapHeaders($headers);

  // we can now run our call. See the WSDL for function definitions
  // This example roughly translates to user_load(), with
  // the param being the UID, same as the same as core's
  // user_load()
  $user_97 = $client->__soapCall('user_soap_retrieve', array(97));

  $functions = $client->__getFunctions();

  print $user_97->name; // prints 'Phillip Kynaston'
} catch (SoapFault $sf) {
  // process $sf error
}
</code>

This code can be run from any PHP script and does not need to be
 called from within a Drupal module.