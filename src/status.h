#ifndef STATUS_H
#define STATUS_H

/**
 * @brief   Project status code enumeration. 
 * @details Shall be used as return values for the project API's
 */
typedef enum eStatus
{
    eSTATUS_SUCCESSFUL,     /**< Operation complete successfully              */
    eSTATUS_NULL_PARAM,     /**< Null parameter has been passed to the API    */
    eSTATUS_INVALID_VALUE,  /**< Invalid parameter has been passed to the API */
    eSTATUS_SYSTEM_ERROR,   /**< Calling the API caused a system error        */
    eSTATUS_ACTION_FAILED,  /**< Requested action failed                      */
    eSTATUS_INVALID_CONFIG, /**< Invalid configuration                        */
    eSTATUS_DEVICE_ERROR    /**< Device error occurred                        */
} eStatus;

#endif