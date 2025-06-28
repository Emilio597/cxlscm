# CXL Persistent Memory Firmware  
  
A CXL persistent memory device firmware implementation based on Eclipse ThreadX RTOS.  
  
## Overview  
  
This project implements a complete CXL persistent memory device firmware with frontend-backend separation architecture:  
- **Frontend (FE)**: Handles CXL protocol processing, host interaction, and device state management  
- **Backend (BE)**: Manages storage media, address translation, and data persistence  
  
## Architecture Features  
  
- Multi-threaded architecture based on ThreadX RTOS  
- State machine-driven frontend control logic  
- Frontend-backend message queue communication  
- Complete CXL.mem protocol support  

## Supported Architecture  
- **ARC HS**: Single-core and SMP multi-core configurations supported  
- **Toolchain**: MetaWare Development Toolkit  
- **ThreadX Version**: 6.4.2
  
## Directory Structure

```
├── fe/ # Frontend implementation
│ ├── src/ # Frontend source code
│ └── inc/ # Frontend headers
├── be/ # Backend implementation
│ ├── src/ # Backend source code
│ └── inc/ # Backend headers
├── cxlscm/ # Common components
└── threadx/common/ # Renamed for clarity, assuming it's ThreadX core
```
