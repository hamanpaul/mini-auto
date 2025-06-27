
# Python FastAPI Server

This is a simple REST API server built with Python and FastAPI.

这是一个使用 Python 和 FastAPI 构建的简单 REST API 服务器。

## Setup (设置)

1.  **Install dependencies (安装依赖):**

    ```bash
    pip install -r requirements.txt
    ```

## Running the Server (运行服务器)

To run the server, execute the following command in the project's root directory:

在项目的根目录中执行以下命令来运行服务器:

```bash
python main.py
```

The server will start on `http://0.0.0.0:8000`.

服务器将在 `http://0.0.0.0:8000` 上启动。

## Adding New APIs (添加新的 API)

To add a new API endpoint, create a new Python file in the `apis` directory. The server will automatically detect and load it.

要添加新的 API 端点，请在 `apis` 目录中创建一个新的 Python 文件。服务器将自动检测并加载它。

For an example, see `apis/example.py`.

有关示例，请参阅 `apis/example.py`。

## Interactive Documentation (交互式文档)

FastAPI provides automatic interactive documentation for your APIs. Once the server is running, you can access it at:

FastAPI 为您的 API 提供自动交互式文档。服务器运行后，您可以在以下地址访问它：

-   **Swagger UI:** [http://127.0.0.1:8000/docs](http://127.0.0.1:8000/docs)
-   **ReDoc:** [http://127.0.0.1:8000/redoc](http://127.0.0.1:8000/redoc)
