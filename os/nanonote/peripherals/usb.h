/* USB messages */

typedef enum DescriptorType
{
    DeviceDesc = 1,
    ConfigurationDesc = 2,
    StringDesc = 3,
    InterfaceDesc = 4,
    EndpointDesc = 5,
    DeviceQualifierDesc = 6,
    OtherSpeedConfigurationDesc = 7,
    InterfacePowerDesc = 8,
}
DescriptorType;

typedef enum RequestType
{
    OutType = 0,
    InType = 0x80,

    StandardType = 0,
    ClassType = 0x20,
    VendorType = 0x40,

    DeviceRec = 0,
    InterfaceRec = 1,
    EndpointRec = 2,
    OtherRec = 3
}
RequestType;

typedef enum RequestNumber
{
    GetStatus = 0,
    ClearFeature = 1,
    SetFeature = 3,
    SetAddress = 5,         /* OutType | StandardType | DeviceRec */
    GetDescriptor = 6,      /* InType | StandardType | DeviceRec */
    SetDescriptor = 7,
    GetConfiguration = 8,
    SetConfiguration = 9,
    GetInterface = 10,
    SetInterface = 11,
    SynchFrame = 12
}
RequestNumber;

typedef struct Request
{
    uchar requesttype;
    uchar request;
    ushort value;
    ushort index;
    ushort length;
}
Request;

typedef struct DeviceDescriptor
{
    uchar length;
    uchar type;         /* 1 */
    ushort version;
    uchar class;
    uchar subclass;
    uchar protocol;
    uchar maxpacketsize;
    ushort vendor;
    ushort product;
    ushort device;
    uchar manufacturer_idx;
    uchar product_idx;
    uchar serialnumber_idx;
    uchar configurations;
}
DeviceDescriptor;

enum {
    USB_Class_CDC       = 0x02,
    USB_Class_HID       = 0x03,
    USB_Class_Vendor    = 0xff,

    USB_NoSubclass      = 0,
    USB_Subclass_ACM    = 0x02,

    USB_NoProtocol      = 0,
    USB_Protocol_ATCmd  = 0x01,
    USB_Protocol_Vendor = 0xff
};

typedef struct ConfigDescriptor
{
    uchar length;       /* at least 9 bytes */
    uchar type;         /* 2 */
    ushort totallength; /* of all descriptors */
    uchar interfaces;
    uchar configuration;
    uchar configuration_idx;
    uchar attributes;
    uchar maxpower;
}
ConfigDescriptor;

/* Only accessible via ConfigDescriptor (USB 2.0 p267) */
typedef struct InterfaceDescriptor
{
    uchar length;       /* 9 bytes */
    uchar type;         /* 4 */
    uchar number;
    uchar alternate;
    uchar endpoints;
    uchar class;
    uchar subclass;
    uchar protocol;
    uchar index;
}
InterfaceDescriptor;

typedef struct EndpointDescriptor
{
    uchar length;       /* 7 bytes */
    uchar type;         /* 5 */
    uchar endpoint;
    uchar attributes;
    uchar max_packet_size_low;
    uchar max_packet_size_high;
    uchar interval;
}
EndpointDescriptor;

enum {
    Endpoint_IN     = 0x80,
    Endpoint_OUT    = 0x00,
    Endpoint_Ctrl   = 0,
    Endpoint_Iso    = 1,
    Endpoint_Bulk   = 2,
    Endpoint_Intr   = 3
};

typedef struct StringDescriptor
{
    uchar length;       /* at least 2 bytes */
    uchar type;         /* 3 */
    ushort string[];    /* UTF-16 */
}
StringDescriptor;
