# Developer Portal Codebase

This repo stores the source code for articles and workshops on the [Espressif Developer Portal](https://developer.espressif.com/). The code mainly includes examples, demos, tests, etc.

Avoid including non-code materials, such as Hugo site source code, media assets, or other supplementary materials.

Each folder under `content/` corresponds to the respective content folder in the [developer-portal](https://github.com/espressif/developer-portal) repo:

```sh
# developer-portal-codebase
📂 content/
├── 📂 blog/
│   └── 📂 article-1/          # code for article-1
│       ├── 📝 README.md           # brief intro
│       ├── 📜 source-code.c
│       └── ...
└── 📂 workshops/
    └── 📂 workshop-1/         # code for workshop-1
        ├── 📝 README.md           # brief intro
        └── 📂 assignment_1_1/
            ├── 📜 main.c
            └── ...
```

The `README.md` file in each content folder should contain configuration options, build steps, and a link to the relevant article.


## Usage

1. Clone this repository locally:
    ```sh
    git clone https://github.com/espressif/developer-portal-codebase.git
    cd developer-portal-codebase
    ```
2. Locate the corresponding source code folder by matching the article’s URL slug:
    ```sh
    # Example URL
    https://developer.espressif.com/ + blog/2025/09/espressif_logging/
    ```
3. Navigate to the respective source code folder:
    ```sh
    # Corresponding path in this repo
    content/ + blog/2025/09/espressif_logging/
    ```


## Future work

- Currently, code blocks are inserted in the Developer Portal articles directly. This is suboptimal, because there is no way to validate or test such code blocks. Instead, source code should be committed to this repo, the code blocks to be included in a Developer Portal article should be marked, then a Hugo shortcode should fetch the code blocks during the build.
- Add a workflow (manual or CI) to validate or test the existing source code. Validation can be done against a stable or specific version of a framework, library, etc.
- Implement a CI check to ensure that the `content/` folder tree is in sync with the `developer-portal` repo. This will ensure that the content paths and source code keep aligned.


## Contributing

- Pull requests and issue reports are welcome!
- If you spot a bug or inconsistency between the article and the code, feel free to open an issue or PR.


## Related links

- [Developer Portal repo](https://github.com/espressif/developer-portal)
- [Developer Portal website](https://developer.espressif.com)
