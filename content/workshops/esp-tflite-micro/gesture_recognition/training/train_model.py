import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from tensorflow import keras


def extract_data():
    o_data = pd.read_csv("./shuttle_dataset/o.csv", header=None)
    v_data = pd.read_csv("./shuttle_dataset/v.csv", header=None)
    unknown_data = pd.read_csv("./shuttle_dataset/unknown.csv", header=None)
    new_gesture_data = pd.read_csv(
        "./shuttle_dataset/new_gesture.csv",
        header=None,
    )

    o_label = np.zeros(o_data.shape[0], dtype=int)
    v_label = np.ones(v_data.shape[0], dtype=int)
    unknown_label = np.full(unknown_data.shape[0], 2, dtype=int)
    new_gesture_label = np.full(new_gesture_data.shape[0], 3, dtype=int)

    X_raw = np.vstack([
        o_data.values,
        v_data.values,
        unknown_data.values,
        new_gesture_data.values,
    ])
    y = np.concatenate([
        o_label,
        v_label,
        unknown_label,
        new_gesture_label,
    ])

    print(f"Counterclockwise circle samples: {o_label.shape[0]}")
    print(f"V gesture samples: {v_label.shape[0]}")
    print(f"Unknown samples: {unknown_label.shape[0]}")
    print(f"New gesture samples: {new_gesture_label.shape[0]}")
    print(f"Total samples: {len(y)}")

    X = X_raw.reshape(X_raw.shape[0], 200, 3)
    return X, y


def prepare_data():
    X, y = extract_data()

    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.int32)

    X_train, X_test, y_train, y_test = train_test_split(
        X,
        y,
        test_size=0.3,
        random_state=42,
    )

    print(f"Training set: {X_train.shape[0]} samples")
    print(f"Test set: {X_test.shape[0]} samples")

    return X_train, X_test, y_train, y_test


def create_1d_cnn_model():
    return keras.Sequential([
        keras.layers.Conv1D(
            filters=8,
            kernel_size=5,
            padding="same",
            activation="relu",
            input_shape=(200, 3),
        ),
        keras.layers.MaxPooling1D(pool_size=4),
        keras.layers.Conv1D(
            filters=16,
            kernel_size=5,
            padding="same",
            activation="relu",
        ),
        keras.layers.MaxPooling1D(pool_size=4),
        keras.layers.GlobalAveragePooling1D(),
        keras.layers.Dense(32, activation="relu"),
        keras.layers.Dropout(0.2),
        keras.layers.Dense(4, activation="softmax"),
    ])


def train_model(model, X_train, y_train, X_test, y_test, epochs=300):
    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=0.001),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"],
    )

    model.summary()

    return model.fit(
        X_train,
        y_train,
        validation_data=(X_test, y_test),
        epochs=epochs,
        batch_size=64,
        verbose=1,
        shuffle=True,
    )


def plot_training_curves(history, save_path="training_curves.png"):
    fig, axes = plt.subplots(1, 2, figsize=(12, 4))
    epochs = range(1, len(history.history["loss"]) + 1)

    axes[0].plot(epochs, history.history["loss"], label="Train Loss")
    axes[0].plot(
        epochs,
        history.history["val_loss"],
        label="Validation Loss",
    )
    axes[0].set_xlabel("Epoch")
    axes[0].set_ylabel("Loss")
    axes[0].set_title("Training and Validation Loss")
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)

    axes[1].plot(
        epochs,
        history.history["accuracy"],
        label="Train Accuracy",
    )
    axes[1].plot(
        epochs,
        history.history["val_accuracy"],
        label="Validation Accuracy",
    )
    axes[1].set_xlabel("Epoch")
    axes[1].set_ylabel("Accuracy")
    axes[1].set_title("Training and Validation Accuracy")
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(save_path, dpi=150)
    plt.show()


def save_model_h5(model, filepath):
    model.save(filepath)
    print(f"Model saved to {filepath}")


if __name__ == "__main__":
    keras.utils.set_random_seed(42)

    X_train, X_test, y_train, y_test = prepare_data()
    model = create_1d_cnn_model()
    history = train_model(model, X_train, y_train, X_test, y_test)

    test_loss, test_accuracy = model.evaluate(X_test, y_test, verbose=0)
    print(f"Test loss: {test_loss:.4f}")
    print(f"Test accuracy: {test_accuracy:.4f}")

    # Optional:
    # plot_training_curves(history)

    save_model_h5(model, "simple_1dcnn_model.h5")
    