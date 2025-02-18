set(SOURCES
    main.c
    tokenizer.c
    compiler_engine.c
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})