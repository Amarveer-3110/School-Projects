import pandas as pd
import torch
from torch.utils.data import DataLoader
from sklearn.metrics import accuracy_score, precision_recall_fscore_support, confusion_matrix
from model_utils import RoadConditionDataset, RoadConditionModel

DEVICE = "cuda" if torch.cuda.is_available() else "cpu"
BATCH_SIZE = 16
MODEL_PATH = "road_condition_model.pt"

def evaluate_split(df, mode="test"):
    print(f"[Eval] Preparing dataset for split='{mode}'...")
    dataset = RoadConditionDataset(df, mode=mode)
    print(f"[Eval] {mode} dataset size: {len(dataset)} samples.")

    print(f"[Eval] Creating DataLoader for {mode} split...")
    loader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=False, num_workers=0)
    num_batches = len(loader)
    print(f"[Eval] {mode} DataLoader ready with {num_batches} batches (batch size={BATCH_SIZE}).")

    print(f"[Eval] Initializing model and loading weights from '{MODEL_PATH}'...")
    model = RoadConditionModel(pretrained=False).to(DEVICE)
    model.load_state_dict(torch.load(MODEL_PATH, map_location=DEVICE))
    model.eval()
    print("[Eval] Model loaded and set to eval mode.")

    all_labels = []
    all_preds = []

    print(f"[Eval] Starting forward passes over {mode} split...")
    with torch.no_grad():
        for batch_idx, (imgs, binary_labels, hazard_levels) in enumerate(loader):
            print(f"  [Eval:{mode}] Batch {batch_idx+1}/{num_batches}: moving data to device...")
            imgs = imgs.to(DEVICE)

            print(f"  [Eval:{mode}] Batch {batch_idx+1}/{num_batches}: forward pass...")
            logits_binary, pred_hazard = model(imgs)

            probs = torch.softmax(logits_binary, dim=1)
            preds = probs.argmax(dim=1).cpu().numpy()
            all_preds.extend(preds)
            all_labels.extend(binary_labels.numpy())

    print(f"[Eval] Finished forward passes for {mode} split. Computing metrics...")

    acc = accuracy_score(all_labels, all_preds)
    precision, recall, f1, _ = precision_recall_fscore_support(
        all_labels, all_preds, average="binary", zero_division=0
    )
    cm = confusion_matrix(all_labels, all_preds)

    print(f"[Eval] Done computing metrics for {mode} split.")
    return acc, precision, recall, f1, cm


def main():
    print("[Step 1] Loading dataframe from disk for evaluation...")
    file_path = r"3P71_dataframe.pkl"
    df = pd.read_pickle(file_path)
    print(f"[Step 1] Dataframe loaded with {len(df)} rows.")

    print("[Step 2] Evaluating on validation split...")
    val_acc, val_prec, val_rec, val_f1, val_cm = evaluate_split(df, mode="val")
    print("[Step 2] Validation metrics:")
    print(f"  Accuracy : {val_acc:.4f}")
    print(f"  Precision: {val_prec:.4f}")
    print(f"  Recall   : {val_rec:.4f}")
    print(f"  F1-score : {val_f1:.4f}")
    print("  Confusion matrix (val):")
    print(val_cm)

    print("\n[Step 3] Evaluating on test split...")
    test_acc, test_prec, test_rec, test_f1, test_cm = evaluate_split(df, mode="test")
    print("[Step 3] Test metrics:")
    print(f"  Accuracy : {test_acc:.4f}")
    print(f"  Precision: {test_prec:.4f}")
    print(f"  Recall   : {test_rec:.4f}")
    print(f"  F1-score : {test_f1:.4f}")
    print("  Confusion matrix (test):")
    print(test_cm)

    print("\n[Done] Evaluation finished for validation and test splits.")


if __name__ == "__main__":
    main()
