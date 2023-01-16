#include "vhitek.h"
namespace VHITEK
{
    namespace Keypad
    {
        MatrixKeyboardManager keyboard;
        MultiIoAbstractionRef ExternalIO = multiIoExpander(0);
        // KeyboardLayout keyLayout(4, 3, "04815926*37#");
        QueueHandle_t KeypadQueue;
        KeyboardLayout keyLayout(4, 4, "DCBA#7410852*963");

        class MyKeyboardListener : public KeyboardListener
        {
        public:
            void keyPressed(char key, bool held) override
            {
                // Serial.printf("key pressed: %c \n", key);
                xQueueSend(KeypadQueue, &key, 10);
            }

            void keyReleased(char key) override
            {
                // Serial.print("key release");
            }

        } myListener;

        void setup()
        {
            multiIoAddExpander(ExternalIO, ioFrom8574(0x38), 8);
            KeypadQueue = xQueueCreate(50, sizeof(char));

            keyLayout.setRowPin(0, 0);
            keyLayout.setRowPin(1, 1);
            keyLayout.setRowPin(2, 2);
            keyLayout.setRowPin(3, 3);

            keyLayout.setColPin(0, 4);
            keyLayout.setColPin(1, 5);
            keyLayout.setColPin(2, 6);
            keyLayout.setColPin(3, 7);

            keyboard.initialise(ExternalIO, &keyLayout, &myListener);
        }

        void loop()
        {
            taskManager.runLoop();
        }

        char getKey()
        {
            char key;
            if (xQueueReceive(KeypadQueue, &key, 2))
            {
                return key;
            }
            return 0;
        }

        void clearKeypad(char *keyData, int lenght)
        {
            for (int i = 0; i < lenght; i++)
            {
                keyData[i] = 0;
            }
            xQueueReset(KeypadQueue);
        }

        void clearKeypad()
        {
            xQueueReset(KeypadQueue);
        }
    }
}