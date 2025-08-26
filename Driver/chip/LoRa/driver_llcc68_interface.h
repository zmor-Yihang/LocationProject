#ifndef DRIVER_LLCC68_INTERFACE_H
#define DRIVER_LLCC68_INTERFACE_H

#include "driver_llcc68.h"
#include "spi/spi.h"
#include "stdarg.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup llcc68_interface_driver llcc68 interface driver function
 * @brief    llcc68 interface driver modules
 * @ingroup  llcc68_driver
 * @{
 */

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t llcc68_interface_spi_init(void);

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t llcc68_interface_spi_deinit(void);

/**
 * @brief      interface spi bus write read
 * @param[in]  *in_buf points to a input buffer
 * @param[in]  in_len is the input length
 * @param[out] *out_buf points to a output buffer
 * @param[in]  out_len is the output length
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t llcc68_interface_spi_write_read(uint8_t *in_buf, uint32_t in_len,
                                        uint8_t *out_buf, uint32_t out_len);

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_init(void);

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_deinit(void);

/**
 * @brief     interface reset gpio write
 * @param[in] data is the written data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t llcc68_interface_reset_gpio_write(uint8_t data);

/**
 * @brief  interface busy gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_init(void);

/**
 * @brief  interface busy gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_deinit(void);

/**
 * @brief      interface busy gpio read
 * @param[out] *value points to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t llcc68_interface_busy_gpio_read(uint8_t *value);

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void llcc68_interface_delay_ms(uint32_t ms);

/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @note      none
 */
void llcc68_interface_debug_print(const char *const fmt, ...);

/**
 * @brief     interface receive callback
 * @param[in] type is the receive callback type
 * @param[in] *buf points to a buffer address
 * @param[in] len is the buffer length
 * @note      none
 */
void llcc68_interface_receive_callback(uint16_t type, uint8_t *buf, uint16_t len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
