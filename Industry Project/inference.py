import io
import pandas as pd
import torch
from torchvision import transforms
from PIL import Image
from model_utils import RoadConditionModel, IMG_SIZE, HAZARD_THRESHOLD

DEVICE = "cuda" if torch.cuda.is_available() else "cpu"
MODEL_PATH = "road_condition_model.pt"

inference_transform = transforms.Compose([
    transforms.Resize((IMG_SIZE, IMG_SIZE)),
    transforms.ToTensor(),
    transforms.Normalize(
        mean=[0.485, 0.456, 0.406],
        std=[0.229, 0.224, 0.225],
    ),
])

def load_model():
    print("[Inference] Initializing model object...")
    model = RoadConditionModel(pretrained=False).to(DEVICE)
    print(f"[Inference] Loading weights from '{MODEL_PATH}'...")
    model.load_state_dict(torch.load(MODEL_PATH, map_location=DEVICE))
    model.eval()
    print("[Inference] Model loaded and set to eval mode.")
    return model

def infer_single_image_bytes(model, img_bytes, idx=None):
    prefix = f"[Inference:sample {idx}]" if idx is not None else "[Inference:single]"
    print(f"{prefix} Decoding image bytes and applying transforms...")
    img = Image.open(io.BytesIO(img_bytes)).convert("RGB")
    img_t = inference_transform(img).unsqueeze(0).to(DEVICE)

    print(f"{prefix} Running forward pass...")
    with torch.no_grad():
        logits_binary, hazard_score = model(img_t)
        probs = torch.softmax(logits_binary, dim=1)[0]
        pred_class = probs.argmax().item()
        prob_hazardous = probs[1].item()
        hazard_score = hazard_score.item()

    label = "hazardous" if pred_class == 1 else "safe"
    print(f"{prefix} Inference complete. Label={label}, hazard_score={hazard_score:.4f}")
    return {
        "label": label,
        "binary_class": int(pred_class),
        "prob_safe": float(probs[0].item()),
        "prob_hazardous": float(prob_hazardous),
        "hazard_score": float(hazard_score),
    }

def main():
    print("[Step 1] Loading dataframe for inference...")
    file_path = r"3P71_dataframe.pkl"
    df = pd.read_pickle(file_path)
    print(f"[Step 1] Dataframe loaded with {len(df)} rows.")

    print("[Step 2] Loading trained model for inference...")
    model = load_model()

    print("[Step 3] Selecting subset of test data for demo inference...")
    test_df = df[df["TrainingID"] == 2].reset_index(drop=True)
    n_samples = min(200, len(test_df))
    print(f"[Step 3] Using first {n_samples} samples from test split for inference.")

    for idx in range(n_samples):
        row = test_df.iloc[idx]
        print(f"\n[Step 4] Running inference on test sample {idx+1}/{n_samples}...")
        img_bytes = row["ImageBytes"] if "ImageBytes" in test_df.columns else row[1]
        result = infer_single_image_bytes(model, img_bytes, idx=idx)
        file_name = row.get("FileName", row.get(0, "unknown_file"))
        print(f"[Result] Sample {idx+1}: File={file_name} -> {result}")

    print("\n[Done] Inference demo finished for selected test samples.")


if __name__ == "__main__":
    main()
