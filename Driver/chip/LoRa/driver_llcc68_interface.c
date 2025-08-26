#include "driver_llcc68_interface.h"
#include "gpio/gpio.h"

#define CS_L (HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET))
#define CS_H (HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET))

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t llcc68_interface_spi_init(void)
{
    SPI_Init();
    GPIOB1_Init();  /* ��ʼ��LoRaоƬBUSY����, ֻ�� */
    GPIOB2_Init();  /* ��ʼ��LoRaоƬTxEN����, дʹ��, Ĭ��ʧ�� */
    GPIOB12_Init(); /* ��ʼ��LoRaоƬRxEN����, ��ʹ��, Ĭ��ʧ�� */
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t llcc68_interface_spi_deinit(void)
{
    HAL_StatusTypeDef result = HAL_SPI_DeInit(&hspi1);
    return result == HAL_OK ? 0 : 1;
}

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
                                        uint8_t *out_buf, uint32_t out_len)
{
    // 1. ��Ƭѡ
    CS_L;

    // 2. ��������
    if (in_len > 0)
    {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, in_buf, in_len, 1000);
        if (status != HAL_OK)
        {
            // ���ͽ���ʧ��  Ҳ��Ҫ�ر�Ƭѡ
            CS_H;
            return 1;
        }
    }
    // 3. ��������
    if (out_len > 0)
    {
        HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, out_buf, out_len, 1000);
        if (status != HAL_OK)
        {
            // ���ͽ���ʧ��  Ҳ��Ҫ�ر�Ƭѡ
            CS_H;
            return 1;
        }
    }

    // �ر�Ƭѡ
    CS_H;
    return 0;
}

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_init(void)
{
    GPIOB0_Init();  /* ��ʼ��LoRaоƬRST����, ��λ */
    return 0;
}

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_deinit(void)
{
    // û��Ҫע��һ������
    return 0;
}

/**
 * @brief     interface reset gpio write
 * @param[in] data is the written data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t llcc68_interface_reset_gpio_write(uint8_t data)
{
    if (data == 0)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    }

    return 0;
}

/**
 * @brief  interface busy gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_init(void)
{
    return 0;
}

/**
 * @brief  interface busy gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief      interface busy gpio read
 * @param[out] *value points to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t llcc68_interface_busy_gpio_read(uint8_t *value)
{
    *value = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void llcc68_interface_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @note      none
 */
void llcc68_interface_debug_print(const char *const fmt, ...)
{
    // �����ɱ�����ṹ�� => �ܹ�����...����
    va_list args;
    // ���÷������տɱ��������
    va_start(args, fmt);
    // ֱ�ӵ��÷�����ӡ => ���ɱ�����Ĵ�ӡ���
    vprintf(fmt, args);
    // �ֶ��ͷ�����ڴ�
    va_end(args);
}

/**
 * @brief     interface receive callback
 * @param[in] type is the receive callback type
 * @param[in] *buf points to a buffer address
 * @param[in] len is the buffer length
 * @note      none
 */
void llcc68_interface_receive_callback(uint16_t type, uint8_t *buf, uint16_t len)
{
    switch (type)
    {
    case LLCC68_IRQ_TX_DONE:
    {
        llcc68_interface_debug_print("llcc68: irq tx done.\n");

        break;
    }
    case LLCC68_IRQ_RX_DONE:
    {
        llcc68_interface_debug_print("llcc68: irq rx done.\n");

        break;
    }
    case LLCC68_IRQ_PREAMBLE_DETECTED:
    {
        llcc68_interface_debug_print("llcc68: irq preamble detected.\n");

        break;
    }
    case LLCC68_IRQ_SYNC_WORD_VALID:
    {
        llcc68_interface_debug_print("llcc68: irq valid sync word detected.\n");

        break;
    }
    case LLCC68_IRQ_HEADER_VALID:
    {
        llcc68_interface_debug_print("llcc68: irq valid header.\n");

        break;
    }
    case LLCC68_IRQ_HEADER_ERR:
    {
        llcc68_interface_debug_print("llcc68: irq header error.\n");

        break;
    }
    case LLCC68_IRQ_CRC_ERR:
    {
        llcc68_interface_debug_print("llcc68: irq crc error.\n");

        break;
    }
    case LLCC68_IRQ_CAD_DONE:
    {
        llcc68_interface_debug_print("llcc68: irq cad done.\n");

        break;
    }
    case LLCC68_IRQ_CAD_DETECTED:
    {
        llcc68_interface_debug_print("llcc68: irq cad detected.\n");

        break;
    }
    case LLCC68_IRQ_TIMEOUT:
    {
        llcc68_interface_debug_print("llcc68: irq timeout.\n");

        break;
    }
    default:
    {
        llcc68_interface_debug_print("llcc68: unknown code.\n");

        break;
    }
    }
}
