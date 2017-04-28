
#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"
#include "string.h"
#include "glyphs.h"
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

unsigned char string_buffer[64];

unsigned int ux_step;
unsigned int ux_step_count;
unsigned int demo_counter;
ux_state_t ux;

typedef struct internalStorage_t {
// #define STORAGE_MAGIC 0xDEAD1337
//     uint32_t magic;
    uint32_t dont_confirm_login;
    uint32_t dynamic_lock;
} internalStorage_t;

WIDE internalStorage_t N_storage_real;
#define N_storage (*(WIDE internalStorage_t *)PIC(&N_storage_real))

// enum app_state_e{
//   unregistered,
//   registered
// };
// typedef enum app_state_e app_state_t;

#define DERIVE_PATH         "72'/69/76/76/79" //HELLO
#define DERIVE_PATH_LEN     (sizeof(DERIVE_PATH)-1)
#define DEVICE_KEY_STR      "Device"
#define DEVICE_KEY_STR_LEN  (sizeof(DEVICE_KEY_STR)-1)
#define AUTH_KEY_STR      "Auth"
#define AUTH_KEY_STR_LEN  (sizeof(AUTH_KEY_STR)-1)
#define DEVICE_GUID_STR     "ID"
#define DEVICE_GUID_STR_LEN (sizeof(DEVICE_GUID_STR)-1)

#define SW_OK 0x9000
#define SW_CONDITIONS_NOT_SATISFIED 0x6985
#define LOGIN_DENIED_BY_USER 0x6984

unsigned char secret_computed;
unsigned char derived_key[32];
unsigned char device_key[32];
unsigned char auth_key[32];
unsigned char device_id[32];
unsigned char nonce_srv[32];
uint8_t refreshUi;
uint8_t replySize;


const ux_menu_entry_t ui_idle_mainmenu[];
const bagl_element_t* ui_idle_menu_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element);
void ui_idle_init(void);

// void ui_idle_submenu_action(unsigned int value) {
//   if (value == 0) {
//     demo_counter = 0;  
//   }
//   else {
//     demo_counter += value;
//   }
//   // redisplay first entry of the idle menu
//   UX_MENU_DISPLAY(0, ui_idle_mainmenu, ui_idle_menu_preprocessor);
// }

// const ux_menu_entry_t ui_idle_submenu[] = {
//   {NULL, ui_idle_submenu_action,    0, NULL, "Reset", NULL, 0, 0},
//   {NULL, ui_idle_submenu_action,   10, NULL, "+10", NULL, 0, 0},
//   {NULL, ui_idle_submenu_action,  100, NULL, "+100", NULL, 0, 0},
//   {NULL, ui_idle_submenu_action, 1000, NULL, "+1000", NULL, 0, 0},
//   {ui_idle_mainmenu, NULL, 1 /*target entry in the referenced menu*/, &C_badge_back, "Back", NULL, 61, 40},
//   UX_MENU_END
// };

// void ui_idle_click(unsigned int value) {
//   demo_counter ++;
// }

const bagl_element_t ui_confirm_registration_nanos[] = {
    // type                               userid    x    y   w    h  str rad
    // fill      fg        bg      fid iid  txt   touchparams...       ]
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x01, 33, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Confirm",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x01, 34, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Registration",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

};

const bagl_element_t ui_confirm_login_nanos[] = {
    // type                               userid    x    y   w    h  str rad
    // fill      fg        bg      fid iid  txt   touchparams...       ]
    {{BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF,
      0, 0},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_LABELINE, 0x01, 33, 12, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Confirm",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},
    {{BAGL_LABELINE, 0x01, 34, 26, 128, 32, 0, 0, 0, 0xFFFFFF, 0x000000,
      BAGL_FONT_OPEN_SANS_EXTRABOLD_11px, 0},
     "Log-in",
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CROSS},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

    {{BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
      BAGL_GLYPH_ICON_CHECK},
     NULL,
     0,
     0,
     0,
     NULL,
     NULL,
     NULL},

};

//unsigned int ui_confirm_registration_nanos_button(unsigned int button_mask,unsigned int button_mask_counter);

unsigned int ui_confirm_registration_nanos_button(unsigned int button_mask,unsigned int button_mask_counter) {
    uint8_t replySize;
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        // confirm
        memcpy(G_io_apdu_buffer,device_key,32);
        memcpy(G_io_apdu_buffer+32,auth_key,32);
        G_io_apdu_buffer[32+32] = SW_OK >> 8;
        G_io_apdu_buffer[32+32+1] = SW_OK & 0xff;
        replySize = 32+32+2;
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, replySize);
        ui_idle_init();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        // deny
        G_io_apdu_buffer[0] = SW_CONDITIONS_NOT_SATISFIED >> 8;
        G_io_apdu_buffer[1] = SW_CONDITIONS_NOT_SATISFIED & 0xff;
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
        ui_idle_init();
        break;

    default:
        break;
    }
}

void compute_device_secrets(void) {
  if (!secret_computed) {
    os_perso_derive_node_bip32(CX_CURVE_SECP256K1, DERIVE_PATH, DERIVE_PATH_LEN, derived_key, NULL);
    cx_sha256_t context;
    // get device_key from hash(DEVICE_KEY_STR,derived_key)
    cx_sha256_init(&context);
    cx_hash(&context,0,DEVICE_KEY_STR,DEVICE_KEY_STR_LEN,NULL);
    cx_hash(&context,CX_LAST,derived_key,32,device_key);
    // get auth_key from hash(AUTH_KEY_STR,derived_key)
    cx_sha256_init(&context);
    cx_hash(&context,0,AUTH_KEY_STR,AUTH_KEY_STR_LEN,NULL);
    cx_hash(&context,CX_LAST,derived_key,32,auth_key);
    // get device_id from hash(DEVICE_GUID_STR,derived_key)
    cx_sha256_init(&context);
    cx_hash(&context,0,DEVICE_GUID_STR,DEVICE_GUID_STR_LEN,NULL);
    cx_hash(&context,CX_LAST,derived_key,32,device_id);
  }
  secret_computed=1;
}

unsigned int compute_login_reply(void) {    
  cx_hmac_sha256_t hmac_context;
  unsigned char device_hmac[32];
  unsigned int rx = 0;

  // confirm
  // Command APDU:
  // ------------- 
  // header (5)
  // HMACsrv (32)
  // NonceSk (32)
  // NonceDk (32)
  //
  // Response APDU:
  // --------------
  // HMACdk (32)
  // HMACsk (32)
  // 1/ Validate deviceHMAC = HMAC(auth_key, nonce_srv || NonceDk || NonceSk)
  cx_hmac_sha256_init(&hmac_context,auth_key,32);         
  cx_hmac(&hmac_context,0,      nonce_srv,32,NULL);
  cx_hmac(&hmac_context,0,      G_io_apdu_buffer+5+32+32,32,NULL);
  cx_hmac(&hmac_context,CX_LAST,G_io_apdu_buffer+5+32,32,device_hmac);
  os_xor(device_hmac, G_io_apdu_buffer+5, device_hmac, 32);
  for (rx=1; rx < 32; rx++) {
    device_hmac[rx] |= device_hmac[rx-1];
  }
  if (device_hmac[31] != 0 || rx != 32) {
    G_io_apdu_buffer[0] = SW_CONDITIONS_NOT_SATISFIED >> 8; // Add code to indicate that HMAC is wrong
    G_io_apdu_buffer[1] = SW_CONDITIONS_NOT_SATISFIED & 0xff;
    return 2;
  }
  // 2/ Compute HMACdk = HMAC(device_key, NonceDk)
  cx_hmac_sha256(device_key,32,G_io_apdu_buffer+5+32+32,32,G_io_apdu_buffer);
  // 3/ Compute HMACsk = HMAC(auth_key, HMACdk || NonceSk)   
  cx_hmac_sha256_init(&hmac_context,auth_key,32);         
  cx_hmac(&hmac_context,0,      G_io_apdu_buffer,32,NULL);
  cx_hmac(&hmac_context,CX_LAST,G_io_apdu_buffer+5+32,32,G_io_apdu_buffer+32);
  G_io_apdu_buffer[32+32] = SW_OK >> 8;
  G_io_apdu_buffer[32+32+1] = SW_OK & 0xff;
  return 32+32+2;
}

unsigned int ui_confirm_login_nanos_button(unsigned int button_mask,unsigned int button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:       
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, compute_login_reply());
        ui_idle_init();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        // deny
        G_io_apdu_buffer[0] = LOGIN_DENIED_BY_USER >> 8;
        G_io_apdu_buffer[1] = LOGIN_DENIED_BY_USER & 0xff;
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
        ui_idle_init();
        break;

    default:
        break;
    }
} 

const ux_menu_entry_t menu_settings[];
const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {ui_idle_mainmenu, NULL, 1, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};                                              

// change the setting
void menu_settings_confirm_login_change(uint32_t confirm) {
  nvm_write(&N_storage.dont_confirm_login, (void*)&confirm, sizeof(uint32_t));
  // go back to the menu entry
  UX_MENU_DISPLAY(0, menu_settings, NULL);
}

// change the setting
void menu_settings_dlock_change(uint32_t confirm) {
  nvm_write(&N_storage.dynamic_lock, (void*)&confirm, sizeof(uint32_t));
  // go back to the menu entry
  UX_MENU_DISPLAY(1, menu_settings, NULL);
}

const ux_menu_entry_t menu_settings_confirm_login[] = {
  {NULL, menu_settings_confirm_login_change, 1, NULL, "No", NULL, 0, 0},
  {NULL, menu_settings_confirm_login_change, 0, NULL, "Yes", NULL, 0, 0},
  UX_MENU_END
};

const ux_menu_entry_t menu_settings_dlock[] = {
  {NULL, menu_settings_dlock_change, 0, NULL, "No", NULL, 0, 0},
  {NULL, menu_settings_dlock_change, 1, NULL, "Yes", NULL, 0, 0},
  UX_MENU_END
};

// show the currently activated entry
void menu_settings_confirm_login_init(unsigned int ignored) {
  UNUSED(ignored);
  UX_MENU_DISPLAY(!N_storage.dont_confirm_login, menu_settings_confirm_login, NULL);
}

// show the currently activated entry
void menu_settings_dlock_init(unsigned int ignored) {
  UNUSED(ignored);
  UX_MENU_DISPLAY(N_storage.dynamic_lock, menu_settings_dlock, NULL);
}

const ux_menu_entry_t menu_settings[] = {
  {NULL, menu_settings_confirm_login_init, 0, NULL, "Confirm login", NULL, 0, 0},
  {NULL, menu_settings_dlock_init, 0, NULL, "Lock when unplugged", NULL, 0, 0},
  {ui_idle_mainmenu, NULL, 0, &C_icon_back, "Back", NULL, 61, 40},
  UX_MENU_END
};

const ux_menu_entry_t ui_idle_mainmenu[] = {
  {NULL, NULL, 0, &C_icon_hello, "Ready to", "authenticate", 37, 11},
  {menu_settings, NULL, 0, NULL, "Settings", NULL, 0, 0},
  {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
  {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
  UX_MENU_END
};

void ui_idle_init(void) {
  ux_step = 0;
  ux_step_count = 2;
  UX_MENU_DISPLAY(0, ui_idle_mainmenu, NULL);
  // setup the first screen changing
  UX_CALLBACK_SET_INTERVAL(1000);
}

// const bagl_element_t* ui_idle_menu_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) {
//   // display all entries but the first one which will be altered.
//   if (entry != &ui_idle_mainmenu[0]) {
//     return element;
//   }

//   switch(element->component.userid) {
//     // display the icon only for the ux step 0
//     case 0x10:
//       if (ux_step != 0) {
//         return 0;
//       }
//       break;
//     // alter the text line
//     case 0x20:
//       switch (ux_step) {
//         case 0x00:
//           // nothing to do this is just a mere string (could overload its position by copying it to a mutable buffer and change it and return the mutable buffer address)
//           // only setup the next screen display delay
//           UX_CALLBACK_SET_INTERVAL(1000);
//           element->text = "Hello world";
//           break;

//         case 0x01:
//           // update the displayed string
//           snprintf(string_buffer, 64, "Current counter value: %d", demo_counter);
//           // setup delay before switching to next screen 
//           // next delay is : at least 3sec, or the time for a label scroll round trip + 1 sec (a pause in either sides).
//           UX_CALLBACK_SET_INTERVAL(MAX(3000, 2*(1000+bagl_label_roundtrip_duration_ms(element, 7))));
//           element->component.stroke = 10; // 1 second stop in each way
//           element->component.icon_id = 26; // roundtrip speed in pixel/s
//           element->text = string_buffer;
//           break;
//       }
//       break;
//   }

//   return element;
// } 

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {

  switch(channel&~(IO_FLAGS)) {

  case CHANNEL_KEYBOARD:
    break;

  // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
  case CHANNEL_SPI:
    if (tx_len) {
      io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

      if (channel & IO_RESET_AFTER_REPLIED) {
        reset();
      }
      return 0; // nothing received from the master so far (it's a tx transaction)
    }
    else {
      return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
    }

  default:
    THROW(INVALID_PARAMETER);
  }
  return 0;
}

void sample_main(void) {
  volatile unsigned int rx = 0;
  volatile unsigned int tx = 0;
  volatile unsigned int flags = 0;

  // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only goal is to retrieve APDU.
  // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make sure the io_event is called with a 
  // switch event, before the apdu is replied to the bootloader. This avoid APDU injection faults.
  for (;;) {
    volatile unsigned short sw = 0;

    BEGIN_TRY {
      TRY {
        rx = tx;
        tx = 0; // ensure no race in catch_other if io_exchange throws an error
        rx = io_exchange(CHANNEL_APDU|flags, rx);
        flags = 0;

        // no apdu received, well, reset the session, and reset the bootloader configuration
        if (rx == 0) {
          THROW(0x6982);
        }

        if (G_io_apdu_buffer[0] != 0x80) {
          THROW(0x6E00);
        }

        // unauthenticated instruction
        switch (G_io_apdu_buffer[1]) {
          // get challenge
          case 0x84:
            cx_rng(nonce_srv,32);
            memcpy(G_io_apdu_buffer,nonce_srv,32);
            tx = 32;
            THROW(SW_OK);

          // mutual authenticate
          case 0x82:
            compute_device_secrets();
            if (N_storage.dont_confirm_login) {
              tx = compute_login_reply();
              // status word is appended during compute
              //THROW(SW_OK);
            }
            else {
              flags |= IO_ASYNCH_REPLY;
              UX_DISPLAY(ui_confirm_login_nanos,NULL);
            }
            break;

          case 0xCA: 
            switch (G_io_apdu_buffer[2]) {
              case 0x00: // device_id
                compute_device_secrets();
                memcpy(G_io_apdu_buffer,device_id,16);
                tx = 16;
                THROW(SW_OK);
                break;
              case 0x01:
                // always asks for a user content
                compute_device_secrets();
                flags |= IO_ASYNCH_REPLY;
                UX_DISPLAY(ui_confirm_registration_nanos,NULL);
                break;
              case 0x02: //D-Lock state
                G_io_apdu_buffer[0] = N_storage.dynamic_lock;
                tx = 1;
                THROW(SW_OK);
                break;
              default:
                THROW(0x6B00);
                break;
            }
            // temp for testing
            //tx = 32;

            // Protect by a user consent
            // TODO show user consent
            // UX_DISPLAY(...);
            // flags = IO_FLAGS_REPLY_ASYNCH;
            break;

          case 0xFF: // return to dashboard
            goto return_to_dashboard;

          default:
            THROW(0x6D00);
            break;
        }
      }
      CATCH_OTHER(e) {
        switch(e & 0xF000) {
          case 0x6000:
          case SW_OK:
            sw = e;
            break;
          default:
            sw = 0x6800 | (e&0x7FF);
            break;
        }
        // Unexpected exception => report 
        G_io_apdu_buffer[tx] = sw>>8;
        G_io_apdu_buffer[tx+1] = sw;
        tx += 2;
      }
      FINALLY {
        
      }
    }
    END_TRY;
  }

return_to_dashboard:
  return;
}

void io_seproxyhal_display(const bagl_element_t * element) {
  return io_seproxyhal_display_default(element);
}


unsigned char io_event(unsigned char channel) {

  // can't have more than one tag in the reply, not supported yet.
  switch (G_io_seproxyhal_spi_buffer[0]) {

    case SEPROXYHAL_TAG_FINGER_EVENT:
    		UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
    		break;


    // power off if long push, else pass to the application callback if any
    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;


    default:
        UX_DEFAULT_EVENT();
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:

      UX_DISPLAYED_EVENT({});
      //UX_DISPLAYED_EVENT(display_done(););
      break;
    case SEPROXYHAL_TAG_TICKER_EVENT:
        UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, 
        {
		  // only allow display when not locked of overlayed by an OS UX.
          if (UX_ALLOWED && ux_step_count) {
            // prepare next screen
            ux_step = (ux_step+1)%ux_step_count;
            // redisplay screen
            UX_REDISPLAY(); 
          }
        });
        break;
  }
  // close the event if not done previously (by a display or whatever)
  if (!io_seproxyhal_spi_is_status_sent()) {
    io_seproxyhal_general_status();
  }
  
  // command has been processed, DO NOT reset the current APDU transport
  return 1;
}




void app_exit(void) {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {

        }
    }
    END_TRY_L(exit);
}

__attribute__ ((section(".boot")))
int main(void) {
  
  // exit critical section
  __asm volatile ("cpsie i");

  secret_computed = 0;
  
  // ensure exception will work as planned
  os_boot();

  UX_INIT();

  BEGIN_TRY {
    TRY {
      io_seproxyhal_init();

      // HID // USB_power(1);
      USB_CCID_power(1);

	    ui_idle_init();

      sample_main();
    }
    CATCH_ALL {
      
    }
    FINALLY {

    }
  }
  END_TRY;

  app_exit();
  return 0;
}