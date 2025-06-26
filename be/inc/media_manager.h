/*
 * ============================================================================
 * 文件: be/inc/media_manager.h
 * ============================================================================
 */
#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H
void media_manager_init(void);
void media_patrol_task(ULONG initial_input);
void wear_leveling_task(ULONG initial_input);
#endif /* MEDIA_MANAGER_H */
