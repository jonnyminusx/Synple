import { useEffect, useRef } from 'react'
import { createPortal } from 'react-dom'

interface MenuItem {
  label: string
  action: () => void
  variant?: 'default' | 'danger'
}

interface ContextMenuProps {
  x: number
  y: number
  items: MenuItem[]
  onClose: () => void
}

export function ContextMenu({ x, y, items, onClose }: ContextMenuProps) {
  const menuRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    const handleDown = (e: MouseEvent) => {
      if (menuRef.current && !menuRef.current.contains(e.target as Node)) {
        onClose()
      }
    }
    document.addEventListener('mousedown', handleDown)
    return () => document.removeEventListener('mousedown', handleDown)
  }, [onClose])

  return createPortal(
    <div
      ref={menuRef}
      style={{ position: 'fixed', left: x, top: y, zIndex: 9999 }}
      className="bg-zinc-800 border border-zinc-600 rounded shadow-lg py-1 min-w-[140px]"
      onContextMenu={e => e.preventDefault()}
    >
      {items.map((item, i) => (
        <button
          key={i}
          className={`w-full text-left px-3 py-1 text-[11px] font-mono hover:bg-zinc-600 ${
            item.variant === 'danger' ? 'text-red-400' : 'text-zinc-200'
          }`}
          onMouseDown={e => {
            e.stopPropagation()
            item.action()
            onClose()
          }}
        >
          {item.label}
        </button>
      ))}
    </div>,
    document.body,
  )
}
