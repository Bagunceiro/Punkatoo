#pragma once

#include <Arduino.h>
#include <vector>
#include <map>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

typedef String MQTTTopic;
class MQTTController;

/**
 * @brief Parent class for devices using the MQTT services
 */
class MQTTClientDev
{
public:
    /**
     * @param devName An identifier for the device for reporting purposes
     */
    MQTTClientDev(const String &devName);
    virtual ~MQTTClientDev();
    /**
     * @brief Register with the MQTTController
     */
    void registerMQTT(MQTTController &c);
    /**
     * @brief Handle received message
     */
    virtual void mqttMsgRecd(const String &topic, const String &msg) {}
    /**
     * @brief Return the device's name
     * @return The device's name
     * @todo String to char*
     */
    const String mqttGetName() { return name; }
    /**
     * @brief Get a status message to be published
     *
     * Not all device types need this so the parent
     * implements a null String return for those that don't.
     *
     * @return The status string to be published
     */
    virtual String mqttGetStatus() { return ""; }
    /** @brief Publish the devices status
     *
     * The topic to be published will consist of
     * the standard prefix + "/device name/status"
     */
    virtual void mqttSendStatus();
    /**
     * @brief Forward a MQTT message to the controller for publishing
     *
     * @param topic The topic to publish (will be prefixed with the device name)
     * @param message The message to be published
     * @param retain Should the server retain the message (true for yes)
     */
    virtual void mqttPublish(const String topic, const String message, bool retain = false);
    /**
     * @brief Check if the server is connected
     * @return true if server is connected
     */
    bool mqttConnected();

protected:
    /**
     * @brief pointer to the controller of which we are a client
     */
    MQTTController *pmqttctlr;

private:
    /**
     * @brief subscribe to any relevant MQTT topics
     */
    virtual void subscribeToMQTT() {}

    String topicPrefix;
    String name;
};

/**
 * @brief A local proxy for the MQTT broker.
 *
 * Client devices register with it and it handles the external communications.
 */
class MQTTController
{
public:
    MQTTController();
    virtual ~MQTTController();
    /**
     * @brief (re)connect to MQTT server
     */
    bool init();
    /**
     * @brief Poll the controller (called by app´s main loop)
     * @return State of the connection to the server (true == up)
     * @todo Make MQTTController its own task?
     */
    bool poll(); // Called by the app's main loop - ToDo: make MQTTController its own task?

    /**
     * @brief Callback for receipt of a message from the server.
     *
     * Calls the private object method msgRecd()
     */
    static void rcvCallback(char *fullTopic, byte *payload, unsigned int length);

    /**
     * @brief Register a device's subscription request
     * @param dev The device making the subscribe request
     * @param topic The topic to be subscribed to
     */
    void subscribe(MQTTClientDev *dev, const MQTTTopic &topic);
    /**
     * @brief Publisd an MQTT message on behalf of a device
     * @param topic Topic of the message
     * @param msg Text of the message
     * @param retained Should the server retain the message
     */
    void publish(const char *topic, const char *msg, const bool retained = false);
    /**
     * @brief Iterate the subscription list and subscribe to all requested topics
     */
    void doSubscriptions();
    /**
     * @brief Add a device to the list of clients
     * @param dev The device to be added
     */
    void addClientDev(MQTTClientDev &dev);
    /**
     * @brief Remove a device from the list of clients
     * @param dev The device to be added
     */

    void rmClientDev(MQTTClientDev &dev);
    /**
     * @brief checks if MQTT server is connected
     * @returns true if MQTT server is connected
     */
    bool connected();

private:
    /**
     * @brief Provides context for callbacks to what should only be the unique instance of this class
     */
    static MQTTController *thectlr;

    /**
     * @brief Routes a message received from the server.
     */
    void msgRecd(const String &fullTopic, const String &msg);
    typedef std::vector<MQTTClientDev *> MQTTDevList;

    typedef std::map<MQTTTopic, MQTTDevList> MQTTSubscriptionList;
    /**
     * @brief map of subscription requests to client device for routing messages
     */
    MQTTSubscriptionList subList;
    /**
     * @brief List of client devices of theis controller
     */
    MQTTDevList devList;
    WiFiClient wifiClient;
    PubSubClient *client;
    /**
     * @brief Returns the prefix that all requests via this controller use
     */
    String stdPrefix();

    /**
     * set by poll to avoid using the thread unsafe PubSubClient::connected elsewhere
     */
    bool connFlag;
};