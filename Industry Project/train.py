import os
import pandas as pd
import torch
import torch.nn as nn
from torch.utils.data import DataLoader
from model_utils import RoadConditionDataset, RoadConditionModel

DEVICE = "cuda" if torch.cuda.is_available() else "cpu"
BATCH_SIZE = 16
EPOCHS = 10
LR = 1e-4
MODEL_PATH = "road_condition_model.pt"  # trained model file

def train_one_epoch(model, loader, optimizer, cls_criterion, reg_criterion, epoch_idx, num_epochs):
    model.train()
    total_cls_loss = 0.0
    total_reg_loss = 0.0
    total = 0

    num_batches = len(loader)
    print(f"  [Epoch {epoch_idx+1}/{num_epochs}] Starting training loop over {num_batches} batches...")

    for batch_idx, (imgs, binary_labels, hazard_levels) in enumerate(loader):
        # Step: prepare batch
        print(f"    [Epoch {epoch_idx+1}/{num_epochs}] Batch {batch_idx+1}/{num_batches}: loading data to device...")
        imgs = imgs.to(DEVICE)
        binary_labels = binary_labels.to(DEVICE)
        hazard_levels = hazard_levels.to(DEVICE)

        optimizer.zero_grad()

        # Step: forward pass
        print(f"    [Epoch {epoch_idx+1}/{num_epochs}] Batch {batch_idx+1}/{num_batches}: forward pass...")
        logits_binary, pred_hazard = model(imgs)

        # Step: compute losses
        loss_cls = cls_criterion(logits_binary, binary_labels)
        loss_reg = reg_criterion(pred_hazard, hazard_levels)
        loss = loss_cls + loss_reg
        print(
            f"    [Epoch {epoch_idx+1}/{num_epochs}] Batch {batch_idx+1}/{num_batches}: "
            f"loss_cls={loss_cls.item():.4f}, loss_reg={loss_reg.item():.4f}"
        )

        # Step: backward + optimize
        print(f"    [Epoch {epoch_idx+1}/{num_epochs}] Batch {batch_idx+1}/{num_batches}: backward + optimize...")
        loss.backward()
        optimizer.step()

        batch_size = imgs.size(0)
        total += batch_size
        total_cls_loss += loss_cls.item() * batch_size
        total_reg_loss += loss_reg.item() * batch_size

    avg_cls_loss = total_cls_loss / total
    avg_reg_loss = total_reg_loss / total
    print(
        f"  [Epoch {epoch_idx+1}/{num_epochs}] Finished training: "
        f"avg_cls_loss={avg_cls_loss:.4f}, avg_reg_loss={avg_reg_loss:.4f}"
    )
    return avg_cls_loss, avg_reg_loss


def validate(model, loader, cls_criterion, reg_criterion, epoch_idx, num_epochs):
    model.eval()
    total_cls_loss = 0.0
    total_reg_loss = 0.0
    total = 0
    correct = 0

    num_batches = len(loader)
    print(f"  [Epoch {epoch_idx+1}/{num_epochs}] Starting validation over {num_batches} batches...")

    with torch.no_grad():
        for batch_idx, (imgs, binary_labels, hazard_levels) in enumerate(loader):
            print(f"    [Epoch {epoch_idx+1}/{num_epochs}] Val batch {batch_idx+1}/{num_batches}: loading data...")
            imgs = imgs.to(DEVICE)
            binary_labels = binary_labels.to(DEVICE)
            hazard_levels = hazard_levels.to(DEVICE)

            print(f"    [Epoch {epoch_idx+1}/{num_epochs}] Val batch {batch_idx+1}/{num_batches}: forward pass...")
            logits_binary, pred_hazard = model(imgs)

            loss_cls = cls_criterion(logits_binary, binary_labels)
            loss_reg = reg_criterion(pred_hazard, hazard_levels)

            probs = torch.softmax(logits_binary, dim=1)
            preds = probs.argmax(dim=1)
            correct += (preds == binary_labels).sum().item()

            batch_size = imgs.size(0)
            total += batch_size
            total_cls_loss += loss_cls.item() * batch_size
            total_reg_loss += loss_reg.item() * batch_size

    avg_cls = total_cls_loss / total
    avg_reg = total_reg_loss / total
    accuracy = correct / total
    print(
        f"  [Epoch {epoch_idx+1}/{num_epochs}] Finished validation: "
        f"avg_cls_loss={avg_cls:.4f}, avg_reg_loss={avg_reg:.4f}, accuracy={accuracy:.4f}"
    )
    return avg_cls, avg_reg, accuracy


def main():
    print("[Step 1] Loading dataframe from disk...")
    file_path = r"3P71_dataframe.pkl"
    df = pd.read_pickle(file_path)
    print(f"[Step 1] Dataframe loaded with {len(df)} rows.")

    print("[Step 2] Creating train and validation datasets...")
    train_dataset = RoadConditionDataset(df, mode="train")
    val_dataset = RoadConditionDataset(df, mode="val")
    print(
        f"[Step 2] Train samples: {len(train_dataset)}, "
        f"Val samples: {len(val_dataset)}."
    )

    print("[Step 3] Creating data loaders...")
    train_loader = DataLoader(train_dataset, batch_size=BATCH_SIZE, shuffle=True, num_workers=0)
    val_loader = DataLoader(val_dataset, batch_size=BATCH_SIZE, shuffle=False, num_workers=0)

    print("[Step 4] Initializing model, loss functions, and optimizer...")
    model = RoadConditionModel().to(DEVICE)
    cls_criterion = nn.CrossEntropyLoss()
    reg_criterion = nn.MSELoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=LR)
    print("[Step 4] Initialization complete.")

    best_val_acc = 0.0

    print("[Step 5] Starting training loop...")
    for epoch in range(EPOCHS):
        print(f"\n=== Epoch {epoch+1}/{EPOCHS} ===")
        train_cls_loss, train_reg_loss = train_one_epoch(
            model, train_loader, optimizer, cls_criterion, reg_criterion, epoch_idx=epoch, num_epochs=EPOCHS
        )

        val_cls_loss, val_reg_loss, val_acc = validate(
            model, val_loader, cls_criterion, reg_criterion, epoch_idx=epoch, num_epochs=EPOCHS
        )

        print(
            f"[Epoch {epoch+1}/{EPOCHS}] Summary: "
            f"Train cls={train_cls_loss:.4f}, reg={train_reg_loss:.4f} | "
            f"Val cls={val_cls_loss:.4f}, reg={val_reg_loss:.4f}, acc={val_acc:.4f}"
        )

        # Step: save best model
        if val_acc > best_val_acc:
            best_val_acc = val_acc
            print(f"[Epoch {epoch+1}/{EPOCHS}] New best validation accuracy {best_val_acc:.4f}. Saving model...")
            torch.save(model.state_dict(), MODEL_PATH)
            print(f"[Epoch {epoch+1}/{EPOCHS}] Model saved to {MODEL_PATH}")

    print(f"[Step 6] Training complete. Best validation accuracy: {best_val_acc:.4f}")
    print("[Done] You can now run the evaluation and inference scripts with this trained model.")


if __name__ == "__main__":
    main()
